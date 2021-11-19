#include "cgmath.h"			// slee's simple math library
#define STB_IMAGE_IMPLEMENTATION
#include "cgut2.h"			// slee's OpenGL utility
#include "trackball.h"
#include "assimp_loader.h"
#include "model.h"
#include "map.h"
#include "wall.h"

//*******************************************************************
// forward declarations for freetype text
bool init_text();
void render_text(std::string text, GLint x, GLint y, GLfloat scale, vec4 color, GLfloat dpi_scale = 1.0f);

//*************************************
// global constants
static const char*	window_name = "cgmodel - assimp for loading {obj|3ds} files";
static const char*	vert_shader_path = "shaders/model.vert";
static const char*	frag_shader_path = "shaders/model.frag";
static const char* mesh_warehouse = "mesh/Room/warehouse/warehouse.obj";
static const char*	mesh_hero = "mesh/Hero/robotcleaner.obj";
static const char*  wood_box = "mesh/gimmick/woodbox/woodbox.obj";

//*************************************
static const char* wall_warehouse = "texture/wall_warehouse.jpg";

//*************************************
// common structures
struct camera
{
	vec3	eye = vec3( 0, -50, 200 );
	vec3	at = vec3( 0, 0, 0 );
	vec3	up = vec3( 0, 1, 0 );
	mat4	view_matrix = mat4::look_at( eye, at, up );
		
	float	fovy = PI/4.0f; // must be in radian
	float	aspect_ratio;
	float	dNear = 1.0f;
	float	dFar = 1000.0f;
	mat4	projection_matrix;
};
struct light_t
{
	vec4	position = vec4(0.0f, 0.0f, 30.0f, 1.0f);   // spot light
	vec4	ambient = vec4(0.3f, 0.3f, 0.3f, 1.0f);
	vec4	diffuse = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4	specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
};

struct material_t
{
	vec4	ambient = vec4(0.3f, 0.3f, 0.3f, 1.0f);
	vec4	diffuse = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4	specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float	shininess = 2000.0f;
};

//*************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = cg_default_window_size(); // initial window size

//*************************************
// OpenGL objects
GLuint	program	= 0;	// ID holder for GPU program
GLuint	wall_vertex_array = 0;
GLuint	WALL_warehouse = 0;
//*************************************
// global variables
int		frame = 0;		// index of rendering frames
bool	show_texcoord = false;
bool	b_2d = false;
float	t;
auto	models = std::move(set_pos()); // positions of models
auto	maps = std::move(create_grid());
auto	walls = std::move(set_wall());
int		scene = 0;
//*************************************
// holder of vertices and indices of a unit wall
std::vector<vertex>	unit_wall_vertices;
//*************************************
// scene objects
std::vector<mesh2*>		pMesh;
camera		cam;
trackball	tb;
light_t		light;
material_t	materials;

//*************************************
// view function
mat4 Ortho(float left, float right, float bottom, float top, float dnear, float dfar) {

	mat4 v = {
		2.0f / (right - left), 0, 0, (left + right) / (left - right),
		0, 2.0f / (top - bottom), 0, (bottom + top) / (bottom - top),
		0, 0, 2.0f / (dnear - dfar), (dnear + dfar) / (dnear - dfar),
		0, 0, 0, 1
	};
	return v;
}

// create wall function
std::vector<vertex> create_wall() // importent
{
	std::vector<vertex> v; // origin
	float h = 50.0f;
	float norm = sqrt(1+h*h);
	v.push_back({ vec3(0, 0.5f,-1.0f * h / 2.0f), vec3(0, 1 / norm,-1.0f * h / norm), vec2(0,0) });
	v.push_back({ vec3(0, -0.5f,-1.0f * h / 2.0f), vec3(0, -1.0f / norm,-1.0f * h / norm), vec2(1,0) });
	v.push_back({ vec3(0, -0.5f, h / 2.0f), vec3(0, -1.0f / norm, h / norm), vec2(1,1) });
	v.push_back({ vec3(0, 0.5f, h / 2.0f), vec3(0, 1 / norm, h / norm), vec2(0,1) });

	return v;
}
void update_vertex_buffer(const std::vector<vertex>& vertices)
{
	static GLuint vertex_buffer = 0;	// ID holder for vertex buffer
	static GLuint index_buffer = 0;		// ID holder for index buffer
	// clear and create new buffers
	if (vertex_buffer)	glDeleteBuffers(1, &vertex_buffer);	vertex_buffer = 0;
	if (index_buffer)	glDeleteBuffers(1, &index_buffer);	index_buffer = 0;
	// check exceptions
	if (vertices.empty()) { printf("[error] vertices is empty.\n"); return; }
	// create buffers
	std::vector<uint> indices;
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(1);
	indices.push_back(0);
	indices.push_back(3);
	indices.push_back(2);
	// generation of vertex buffer: use vertices as it is
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	// geneation of index buffer
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);

	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	if (wall_vertex_array) glDeleteVertexArrays(1, &wall_vertex_array);
	wall_vertex_array = cg_create_vertex_array(vertex_buffer, index_buffer);
	if (!wall_vertex_array) { printf("%s(): failed to create vertex aray\n", __func__); return; }
}

//*************************************
void update()
{
	glUseProgram(program);

	// update projection matrix
	cam.aspect_ratio = window_size.x/float(window_size.y);
	if (b_2d) { // 카메라 부분
		mat4 aspect_matrix =
		{
			std::min(1 / cam.aspect_ratio,1.0f), 0, 0, 0,
			0, std::min(cam.aspect_ratio,1.0f), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};
		cam.projection_matrix = aspect_matrix * Ortho(-30.f, 30.f, -10.0f, 40.0f, 10.5f, 400); // 보이는 영역
		cam.view_matrix = mat4::look_at(vec3(50.0f, models[1].center.y, 10), vec3(0, models[1].center.y, 10), vec3( -1, 0, 1 )); // 시점 확정
	}
	else {
		//cam.view_matrix = mat4::look_at(models[1].center+vec3(0, -30, 140), models[1].center, vec3(0, 1, 0));
		cam.projection_matrix = mat4::perspective(cam.fovy, cam.aspect_ratio, cam.dNear, cam.dFar); //보이는 영역
	}

	// build the model matrix for oscillating scale
	t = float(glfwGetTime());
	// update uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation( program, "view_matrix" );			if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, cam.view_matrix );
	uloc = glGetUniformLocation( program, "projection_matrix" );	if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, cam.projection_matrix );
	
	// setup light properties
	glUniform4fv(glGetUniformLocation(program, "light_position"), 1, light.position);
	glUniform4fv(glGetUniformLocation(program, "Ia"), 1, light.ambient);
	glUniform4fv(glGetUniformLocation(program, "Id"), 1, light.diffuse);
	glUniform4fv(glGetUniformLocation(program, "Is"), 1, light.specular);

	// setup material properties
	glUniform4fv(glGetUniformLocation(program, "Ka"), 1, materials.ambient);
	glUniform4fv(glGetUniformLocation(program, "Kd"), 1, materials.diffuse);
	glUniform4fv(glGetUniformLocation(program, "Ks"), 1, materials.specular);
	glUniform1f(glGetUniformLocation(program, "shininess"), materials.shininess);
}

void render()
{
	// clear screen (with background color) and clear depth buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	// notify GL that we use our own program
	glUseProgram( program );
	glActiveTexture(GL_TEXTURE0);
	int i = 0;

	// bind vertex array object
	for (auto& m : models) {
		glBindVertexArray(pMesh[i]->vertex_array);
		m.update(t);
		GLint uloc;
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, m.model_matrix);
		for (size_t k = 0, kn = pMesh[i]->geometry_list.size(); k < kn; k++) {
			geometry& g = pMesh[i]->geometry_list[k];

			
			if (g.mat->textures.diffuse) {
				glBindTexture(GL_TEXTURE_2D, g.mat->textures.diffuse->id);
				glUniform1i(glGetUniformLocation(program, "TEX"), 0);	 // GL_TEXTURE0
				glUniform1i(glGetUniformLocation(program, "use_texture"), true);
				glUniform1i(glGetUniformLocation(program, "mode"), 0);

			}
			else {
				glUniform4fv(glGetUniformLocation(program, "diffuse"), 1, (const float*)(&g.mat->diffuse));
				glUniform1i(glGetUniformLocation(program, "use_texture"), false);
				glUniform1i(glGetUniformLocation(program, "mode"), 0);
			}

			// render vertices: trigger shader programs to process vertex data
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMesh[i]->index_buffer);
			glDrawElements(GL_TRIANGLES, g.index_count, GL_UNSIGNED_INT, (GLvoid*)(g.index_start * sizeof(GLuint)));
		}
		i++;
	}
	glBindVertexArray(wall_vertex_array);
	glActiveTexture(GL_TEXTURE1);								// select the texture slot to bind
	glBindTexture(GL_TEXTURE_2D, WALL_warehouse);
	for (auto& w : walls) {
		w.setSize();
		GLint uloc;
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, w.model_matrix);
		glUniform1i(glGetUniformLocation(program, "TEX"), 1);
		glUniform1i(glGetUniformLocation(program, "use_texture"), true);
		glUniform1i(glGetUniformLocation(program, "mode"), 1);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr); // 변경 여지
	}
	//text render
	float dpi_scale = cg_get_dpi_scale();
	render_text("Hello text!", 50, 50, 1.0f, vec4(0.5f, 0.8f, 0.2f, 1.0f), dpi_scale);
	render_text("I love Computer Graphics!", 100, 125, 0.5f, vec4(0.7f, 0.4f, 0.1f, 0.8f), dpi_scale);
	render_text("Blinking text here", 100, 155, 0.6f, vec4(0.5f, 0.7f, 0.7f, abs(sin(t * 2.5f))), dpi_scale);
	// swap front and back buffers, and display to screen
	glfwSwapBuffers( window );
}

void reshape( GLFWwindow* window, int width, int height )
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width,height);
	glViewport( 0, 0, width, height );
}

void print_help()
{
	printf( "[help]\n" );
	printf( "- press ESC or 'q' to terminate the program\n" );
	printf( "- press F1 or 'h' to see help\n" );
	printf( "- press Home to reset camera\n" );
	printf( "- press 'd' to toggle between OBJ format and 3DS format\n" );
	printf( "\n" );
}

void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if(action==GLFW_PRESS)
	{
		if(key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q)	glfwSetWindowShouldClose( window, GL_TRUE );
		else if(key==GLFW_KEY_H||key==GLFW_KEY_F1)	print_help();
		else if(key==GLFW_KEY_HOME)					cam = camera();
		else if(key==GLFW_KEY_T)					show_texcoord = !show_texcoord;
		else if (key == GLFW_KEY_R)
		{
			b_2d = !b_2d;
		}
		else if (key == GLFW_KEY_A)
		{
			models[1].action = PUSH;
		}
		else if (key == GLFW_KEY_S)
		{
			models[1].action = PULL;
		}
		else if (key == GLFW_KEY_RIGHT) {
			models[1].right_move(maps[scene], models);
		}
		else if (key == GLFW_KEY_LEFT) {
			models[1].left_move(maps[scene], models);
		}
		else if (key == GLFW_KEY_UP) {
			models[1].up_move(maps[scene], models);
		}
		else if (key == GLFW_KEY_DOWN) {
			models[1].down_move(maps[scene], models);
		}
	}

	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_A)
		{
			models[1].action = 0;
		}
		else if (key == GLFW_KEY_S)
		{
			models[1].action = 0;
		}
	}
}

void mouse( GLFWwindow* window, int button, int action, int mods )
{
	if(button==GLFW_MOUSE_BUTTON_LEFT)
	{
		dvec2 pos; glfwGetCursorPos(window,&pos.x,&pos.y);
		vec2 npos = cursor_to_ndc( pos, window_size );
		if(action==GLFW_PRESS)			tb.begin( cam.view_matrix, npos );
		else if(action==GLFW_RELEASE)	tb.end();
	}
}

void motion( GLFWwindow* window, double x, double y )
{
	if(!tb.is_tracking()) return;
	vec2 npos = cursor_to_ndc( dvec2(x,y), window_size );
	cam.view_matrix = tb.update( npos );
}

bool user_init()
{
	// log hotkeys
	print_help();

	// init GL states
	glClearColor( 39/255.0f, 40/255.0f, 34/255.0f, 1.0f );	// set clear color
	glEnable(GL_BLEND);
	glEnable( GL_CULL_FACE );								// turn on backface culling
	glEnable( GL_DEPTH_TEST );								// turn on depth tests
	glEnable( GL_TEXTURE_2D );
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture( GL_TEXTURE0 );

	unit_wall_vertices = std::move(create_wall());
	update_vertex_buffer(unit_wall_vertices);
	WALL_warehouse = cg_create_texture(wall_warehouse, true); if (!WALL_warehouse) return false;


	// load the mesh
	pMesh.emplace_back(load_model(mesh_warehouse));
	pMesh.emplace_back(load_model(mesh_hero));
	pMesh.emplace_back(load_model(wood_box));
	pMesh.emplace_back(load_model(wood_box));

	if(pMesh.empty()){ printf( "Unable to load mesh\n" ); return false; }
	if (!init_text()) return false;
	return true;
}

void user_finalize()
{
	delete_texture_cache();
	pMesh.clear();
}

int main( int argc, char* argv[] )
{
	// create window and initialize OpenGL extensions
	if(!(window = cg_create_window( window_name, window_size.x, window_size.y ))){ glfwTerminate(); return 1; }
	if(!cg_init_extensions( window )){ glfwTerminate(); return 1; }	// version and extensions

	// initializations and validations
	if(!(program=cg_create_program( vert_shader_path, frag_shader_path ))){ glfwTerminate(); return 1; }	// create and compile shaders/program
	if(!user_init()){ printf( "Failed to user_init()\n" ); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback( window, reshape );	// callback for window resizing events
    glfwSetKeyCallback( window, keyboard );			// callback for keyboard events
	glfwSetMouseButtonCallback( window, mouse );	// callback for mouse click inputs
	glfwSetCursorPosCallback( window, motion );		// callback for mouse movement

	// enters rendering/event loop
	for( frame=0; !glfwWindowShouldClose(window); frame++ )
	{
		glfwPollEvents();	// polling and processing of events
		update();			// per-frame update
		render();			// per-frame render
	}
	
	// normal termination
	user_finalize();
	cg_destroy_window(window);

	return 0;
}

