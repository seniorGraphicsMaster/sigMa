#include "assimp_loader.h"

#ifdef _MSC_VER
	#pragma comment( lib, "assimp.lib" ) // link assimp
#endif

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#if defined(_MSC_VER)
	#include <filesystem>
	namespace fs = std::filesystem;
#elif defined(__GNUC__) || defined(__GNUG__)
	// NOTE: compile with options: -lboost_filesystem -lboost_system
	#include <boost/filesystem>
	namespace fs = boost::filesystem;
#endif

// Texture cache.
static std::unordered_map<std::string, Texture*> texture_cache;

// forward decl.
extern ivec2 window_size;

// *******************************************************************
void delete_texture_cache(void)
{
	for( auto& texture: texture_cache ) delete texture.second;
	texture_cache.clear();
}

/*!
 * Load a texture.
 *
 * This function loads a texture from an image file and creates OpenGL texture.
 * Some textures are shared on some material, so we use texture cache for efficiency.
 * Texture cache uses map structure.
 */
Texture* load_texture( const aiMaterial* ai_material, const fs::path& model_directory, const aiTextureType texture_type )
{
	aiString _path;
	if( ai_material->GetTextureCount(texture_type)==1 )
	{
		if(ai_material->GetTexture(texture_type,0,&_path)==aiReturn::aiReturn_FAILURE)
			return nullptr;

		const fs::path& texture_path = model_directory / fs::path(_path.C_Str());

		// check if texture exists, or not.
		if(!fs::exists(texture_path))
		{
			printf("texture [%s] not exists.\n", texture_path.string().c_str());
			return nullptr;
		}

		// key is texture path in material.
		if(texture_cache.find(texture_path.string()) != texture_cache.end())
		{
			return texture_cache[texture_path.string()];
		}

		Texture* tex = new Texture;
		tex->path = texture_path.string();
		tex->id = cg_create_texture( texture_path.string().c_str(), true ); if(!tex->id) return nullptr;

		return texture_cache[texture_path.string()] = tex;
	}

	return nullptr;
}

mesh2* load_model( const char* path, bool bFlipUV )
{
	std::string cpath = std::string(absolute_path(path));
	fs::path model_path(cpath);

	// *** preprocess ************************************************************
	// These flags configure features of assimp loader.
	unsigned int flags =    aiPostProcessSteps::aiProcess_Triangulate |	// Triangulates all faces of all meshes. 
		aiPostProcessSteps::aiProcess_JoinIdenticalVertices |			// Identifies and joins identical vertex data sets within all imported meshes.
		aiPostProcessSteps::aiProcess_GenSmoothNormals |				// Generates smooth normals for all vertices in the mesh.
		aiPostProcessSteps::aiProcess_ImproveCacheLocality |			// Reorders triangles for better vertex cache locality.
		aiPostProcessSteps::aiProcess_FindInstances |					// This step searches for duplicate meshes and replaces them with references to the first mesh.

		aiPostProcessSteps::aiProcess_RemoveRedundantMaterials |		// Searches for redundant/unreferenced materials and removes them.
		// WARN: This flag changes the name of material.

		aiPostProcessSteps::aiProcess_SortByPType;						// This step splits meshes with more than one primitive type in homogeneous sub-meshes.
	if(bFlipUV)
		flags |= aiPostProcessSteps::aiProcess_FlipUVs;					// This step flips all UV coordinates along the y-axis and adjusts material settings and bitangents accordingly.

	Assimp::Importer importer;

	printf("Loading model: %s ... ", path );

	// read and parse a model.
	const aiScene* ai_scene = importer.ReadFile(model_path.string(), flags);
	if(!ai_scene)
	{
		printf("\nAssimp error: %s\n", importer.GetErrorString());
		return nullptr;
	}

	// compute the size of vertices and indices.
	size_t num_verticies = 0;
	size_t num_indicies = 0;
	for( unsigned int mesh_id = 0; mesh_id < ai_scene->mNumMeshes; mesh_id++)
	{
		const aiMesh* ai_mesh = ai_scene->mMeshes[mesh_id];

		num_verticies += ai_mesh->mNumVertices;

		// ignore points and lines.
		if(ai_mesh->mPrimitiveTypes == aiPrimitiveType::aiPrimitiveType_TRIANGLE)
		{
			for(size_t k = 0; k < ai_mesh->mNumFaces; k++)
			{
				num_indicies += ai_mesh->mFaces[k].mNumIndices;
			}
		}
	}

	// *** data transform ********************************************************
	mesh2* new_mesh = new mesh2();
	new_mesh->vertex_list.reserve(num_verticies);
	new_mesh->index_list.reserve(num_indicies);
	new_mesh->material_list.resize(ai_scene->mNumMaterials, nullptr);

	// mesh setup
	std::vector<vertex>& vertex_list = new_mesh->vertex_list;
	std::vector<uint>& index_list = new_mesh->index_list;
	for( unsigned int mesh_id = 0; mesh_id < ai_scene->mNumMeshes; mesh_id++ )
	{
		const aiMesh* ai_mesh = ai_scene->mMeshes[mesh_id];

		// ignore points and lines.
		if(ai_mesh->mPrimitiveTypes != aiPrimitiveType::aiPrimitiveType_TRIANGLE)
			continue;

		// index setup
		const size_t vertex_start = vertex_list.size();
		const size_t index_start = index_list.size();
		for(size_t k = 0; k < ai_mesh->mNumFaces; k++)
		{
			const aiFace& face = ai_mesh->mFaces[k];

			auto& index_begin = index_list.insert(index_list.end(), face.mIndices, face.mIndices + face.mNumIndices);

			// aiMesh's index indicates the index of vertex in "aiMesh".
			// so, we add the starting index in vertex_list.
			const auto& index_end = index_list.end();
			for(; index_begin < index_end; index_begin++)
				*index_begin += vertex_start;
		}

		// vertex setup (pos/norm/tex)
		const vec3* pos = reinterpret_cast<vec3*>(ai_mesh->mVertices);
		const vec3* norm = reinterpret_cast<vec3*>(ai_mesh->mNormals);
		if(ai_mesh->HasTextureCoords(0)) {
			const vec3* tex = reinterpret_cast<vec3*>(ai_mesh->mTextureCoords[0]);
			for(size_t k = 0; k < ai_mesh->mNumVertices; k++)
				vertex_list.push_back(std::move(vertex{ pos[k], norm[k], vec2(tex[k].x, tex[k].y) }));
		}
		else
		{
			for(size_t k = 0; k < ai_mesh->mNumVertices; k++)
				vertex_list.push_back(std::move(vertex{ pos[k], norm[k], vec2(0.0f, 0.0f) }));
		}

		// material setup
		material* mat = nullptr;
		if(ai_mesh->mMaterialIndex >= 0 && (mat = new_mesh->material_list[ai_mesh->mMaterialIndex]) == nullptr)
		{
			mat = new_mesh->material_list[ai_mesh->mMaterialIndex] = new material;
			const aiMaterial* ai_material = ai_scene->mMaterials[ai_mesh->mMaterialIndex];

			aiString mat_name;
			ai_material->Get(AI_MATKEY_NAME, mat_name);
			mat->name.assign(mat_name.C_Str());

			ai_material->Get(AI_MATKEY_COLOR_AMBIENT, reinterpret_cast<aiColor4D&>(mat->ambient));
			ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor4D&>(mat->diffuse));
			ai_material->Get(AI_MATKEY_COLOR_SPECULAR, reinterpret_cast<aiColor4D&>(mat->specular));
			ai_material->Get(AI_MATKEY_COLOR_EMISSIVE, reinterpret_cast<aiColor4D&>(mat->emissive));
			ai_material->Get(AI_MATKEY_SHININESS, mat->shininess);
			ai_material->Get(AI_MATKEY_OPACITY, mat->opacity);
			ai_material->Get(AI_MATKEY_REFRACTI, mat->refractive_index);

			// texture (diffuse only)
			mat->textures.diffuse = load_texture(ai_material, model_path.parent_path(), aiTextureType::aiTextureType_DIFFUSE);
		}

		// append a geometry.
		new_mesh->geometry_list.push_back(std::move(geometry(mat, index_start, index_list.size() - index_start)));
	}

	// *** buffer creation *******************************************************
	// create a vertex buffer
	glGenBuffers(1, &new_mesh->vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, new_mesh->vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(decltype(new_mesh->vertex_list)::value_type)*new_mesh->vertex_list.size(), new_mesh->vertex_list.data(), GL_STATIC_DRAW);

	// create a index buffer
	glGenBuffers(1, &new_mesh->index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_mesh->index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(decltype(new_mesh->index_list)::value_type)*new_mesh->index_list.size(), new_mesh->index_list.data(), GL_STATIC_DRAW);

	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	new_mesh->vertex_array = cg_create_vertex_array( new_mesh->vertex_buffer, new_mesh->index_buffer );
	if(!new_mesh->vertex_array){ printf("%s(): failed to create vertex aray\n",__func__); return nullptr; }

	printf("completed\n");

	return new_mesh;
}
