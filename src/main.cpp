#include "cgmath.h"			// slee's simple math library
#define STB_IMAGE_IMPLEMENTATION
#include "cgut2.h"			// slee's OpenGL utility
#include "trackball.h"
#include "assimp_loader.h"
#include "model.h"

//*************************************
// global constants
static const char*	window_name = "cgmodel - assimp for loading {obj|3ds} files";
static const char*	vert_shader_path = "shaders/model.vert";
static const char*	frag_shader_path = "shaders/model.frag";
//static const char*	mesh_obj = "mesh/head/head.obj";
static const char*	mesh_3ds = "mesh/head/head.3ds";
static const char*	mesh_obj = "mesh/room_Test/map_test.obj";
static const char*	mesh_hero = "mesh/Hero/robotcleaner.obj";

//*************************************
// common structures
struct camera
{
	vec3	eye = vec3( 0, -100, 10 );
	vec3	at = vec3( 0, 0, 0 );
	vec3	up = vec3( 0, 1, 0 );
	mat4	view_matrix = mat4::look_at( eye, at, up );
		
	float	fovy = PI/4.0f; // must be in radian
	float	aspect_ratio;
	float	dNear = 1.0f;
	float	dFar = 1000.0f;
	mat4	projection_matrix;
};

//*************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = cg_default_window_size(); // initial window size

//*************************************
// OpenGL objects
GLuint	program	= 0;	// ID holder for GPU program

//*************************************
// global variables
int		frame = 0;		// index of rendering frames
bool	show_texcoord = false;
bool	b_2d = false;
float	t;
auto	models = std::move(set_pos()); // positions of models

//*************************************
// scene objects
std::vector<mesh2*>		pMesh;
camera		cam;
trackball	tb;
// custom function
mat4 Ortho(float left, float right, float bottom, float top, float dnear, float dfar) {

	mat4 v = {
		2.0f / (right - left), 0, 0, (left + right) / (left - right),
		0, 2.0f / (top - bottom), 0, (bottom + top) / (bottom - top),
		0, 0, 2.0f / (dnear - dfar), (dnear + dfar) / (dnear - dfar),
		0, 0, 0, 1
	};
	return v;
}
//*************************************
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
		cam.projection_matrix = aspect_matrix * Ortho(-400.0f, 400.0f, -400.0f, 400.0f, cam.dNear, cam.dFar);
	}
	else {
		cam.projection_matrix = mat4::perspective(cam.fovy, cam.aspect_ratio, cam.dNear, cam.dFar);
	}

	// build the model matrix for oscillating scale
	t = float(glfwGetTime());
	float scale = 1.0f;
	mat4 model_matrix = mat4::scale( scale, scale, scale );
	// update uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation( program, "view_matrix" );			if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, cam.view_matrix );
	uloc = glGetUniformLocation( program, "projection_matrix" );	if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, cam.projection_matrix );
	uloc = glGetUniformLocation( program, "model_matrix" );			if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, model_matrix );

	glActiveTexture(GL_TEXTURE0);								// select the texture slot to bind
}

void render()
{
	// clear screen (with background color) and clear depth buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	// notify GL that we use our own program
	glUseProgram( program );

	// bind vertex array object
	glBindVertexArray( pMesh[0]->vertex_array );
	models[0].update(t);
	GLint uloc;
	uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, models[0].model_matrix);
	for( size_t k=0, kn= pMesh[0]->geometry_list.size(); k < kn; k++ ) {
		geometry& g = pMesh[0]->geometry_list[k];

		if (g.mat->textures.diffuse) {
			glBindTexture(GL_TEXTURE_2D, g.mat->textures.diffuse->id);
			glUniform1i(glGetUniformLocation(program, "TEX"), 0);	 // GL_TEXTURE0
			glUniform1i(glGetUniformLocation(program, "use_texture"), true);
		} else {
			glUniform4fv(glGetUniformLocation(program, "diffuse"), 1, (const float*)(&g.mat->diffuse));
			glUniform1i(glGetUniformLocation(program, "use_texture"), false);
		}

		// render vertices: trigger shader programs to process vertex data
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, pMesh[0]->index_buffer );
		glDrawElements( GL_TRIANGLES, g.index_count, GL_UNSIGNED_INT, (GLvoid*)(g.index_start*sizeof(GLuint)) );
	}
	glBindVertexArray(pMesh[1]->vertex_array);
	models[1].update(t);
	GLint uloc1;
	uloc1 = glGetUniformLocation(program, "model_matrix");		if (uloc1 > -1) glUniformMatrix4fv(uloc1, 1, GL_TRUE, models[1].model_matrix);
	for (size_t k = 0, kn = pMesh[1]->geometry_list.size(); k < kn; k++) {
		geometry& g = pMesh[1]->geometry_list[k];

		if (g.mat->textures.diffuse) {
			glBindTexture(GL_TEXTURE_2D, g.mat->textures.diffuse->id);
			glUniform1i(glGetUniformLocation(program, "TEX"), 0);	 // GL_TEXTURE0
			glUniform1i(glGetUniformLocation(program, "use_texture"), true);
		}
		else {
			glUniform4fv(glGetUniformLocation(program, "diffuse"), 1, (const float*)(&g.mat->diffuse));
			glUniform1i(glGetUniformLocation(program, "use_texture"), false);
		}

		// render vertices: trigger shader programs to process vertex data
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMesh[1]->index_buffer);
		glDrawElements(GL_TRIANGLES, g.index_count, GL_UNSIGNED_INT, (GLvoid*)(g.index_start * sizeof(GLuint)));
	}

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
		else if(key==GLFW_KEY_D)
		{
			static bool is_obj = true;
			is_obj = !is_obj;

			glFinish();
			delete_texture_cache();
			pMesh.clear();
			pMesh[0] = load_model(is_obj ? mesh_obj : mesh_3ds);
		}
		else if (key == GLFW_KEY_R)
		{
			b_2d = !b_2d;
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
	glEnable( GL_CULL_FACE );								// turn on backface culling
	glEnable( GL_DEPTH_TEST );								// turn on depth tests
	glEnable( GL_TEXTURE_2D );
	glActiveTexture( GL_TEXTURE0 );

	// load the mesh
	pMesh.emplace_back(load_model(mesh_obj));
	pMesh.emplace_back(load_model(mesh_hero));
	if(pMesh.empty()){ printf( "Unable to load mesh\n" ); return false; }

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

