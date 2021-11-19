#include "cgmath.h"			// slee's simple math library
#define STB_IMAGE_IMPLEMENTATION
#include "cgut2.h"			// slee's OpenGL utility
#include "trackball.h"
#include "assimp_loader.h"
#include "model.h"
#include "map.h"

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
static const char* hero_title = "images/hero.png";
//*************************************


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
	vec4	position = vec4(0.0f, 0.0f, 20.0f, 1.0f);   // spot light
	vec4	ambient = vec4(0.2f, 0.2f, 0.2f, 1.0f);
	vec4	diffuse = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4	specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
};

struct material_t
{
	vec4	ambient = vec4(0.2f, 0.2f, 0.2f, 1.0f);
	vec4	diffuse = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4	specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float	shininess = 1000.0f;
};

//*************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = cg_default_window_size(); // initial window size

//*************************************
// OpenGL objects
GLuint	program	= 0;	// ID holder for GPU program
GLuint	TITLE = 0;			// RGB texture object
GLuint vertex_array = 0;
//*************************************
// global variables
int		frame = 0;		// index of rendering frames
bool	show_texcoord = false;
bool	b_2d = false;
float	t;
auto	models = std::move(set_pos()); // positions of models
auto	maps = std::move(create_grid());

int		scene = 0;

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

// move character function


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
		cam.view_matrix = mat4::look_at(models[1].center+vec3(0, -30, 140), models[1].center, vec3(0, 1, 0));
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
	
	if (scene == 0) {
		float dpi_scale = cg_get_dpi_scale();
		render_text("Game Title", 50, 100, 1.0f, vec4(0.5f, 0.8f, 0.2f, 1.0f), dpi_scale);
		render_text("Team sigma - Dongmin, Dongjun, Jiye", 50, 300, 0.5f, vec4(0.7f, 0.4f, 0.1f, 0.8f), dpi_scale);
		render_text("Please 's' to start", 50, 355, 0.6f, vec4(0.5f, 0.5f, 0.5f, abs(sin(t * 2.5f))), dpi_scale);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TITLE);
		glUniform1i(glGetUniformLocation(program, "TEX0"), 0);

		glBindVertexArray(vertex_array);
	}

	if (scene == 1) {
		float dpi_scale = cg_get_dpi_scale();
		
		render_text("My name is zetbot.. I'm vending machine..", 30, 355, 0.5f, vec4(0.7f, 0.4f, 0.1f, 0.8f), dpi_scale);
		render_text("Please press 'n' to next", 30, 400, 0.4f, vec4(0.5f, 0.5f, 0.5f, abs(sin(t * 2.5f))), dpi_scale);
	}

	if (scene == 2) {
		float dpi_scale = cg_get_dpi_scale();

		render_text("I can't live in this house cleaning anymore!", 30, 300, 0.5f, vec4(0.7f, 0.4f, 0.1f, 0.8f), dpi_scale);
		render_text("I'm going to escape!", 30, 355, 0.5f, vec4(0.7f, 0.4f, 0.1f, 0.8f), dpi_scale);
		render_text("Please press 'n' to next", 30, 400, 0.4f, vec4(0.5f, 0.5f, 0.5f, abs(sin(t * 2.5f))), dpi_scale);
	}

	if (scene == 3) {
		float dpi_scale = cg_get_dpi_scale();

		render_text("To escape, move the box in 3D and get the key", 30, 300, 0.5f, vec4(0.7f, 0.4f, 0.1f, 0.8f), dpi_scale);
		render_text("...... ", 30, 355, 0.5f, vec4(0.7f, 0.4f, 0.1f, 0.8f), dpi_scale);
		render_text("Please press 'n' to next", 30, 400, 0.4f, vec4(0.5f, 0.5f, 0.5f, abs(sin(t * 2.5f))), dpi_scale);
	}

	if (scene == 4) {
		float dpi_scale = cg_get_dpi_scale();

		render_text("I have to escape the room within a certain time.", 30, 300, 0.5f, vec4(0.7f, 0.4f, 0.1f, 0.8f), dpi_scale);
		render_text("IF my battery runs out or the owner retuns, I fail.. ", 30, 355, 0.5f, vec4(0.7f, 0.4f, 0.1f, 0.8f), dpi_scale);
		render_text("Please press 'n' to next", 30, 400, 0.4f, vec4(0.5f, 0.5f, 0.5f, abs(sin(t * 2.5f))), dpi_scale);
	}

	if (scene == 5) {
		float dpi_scale = cg_get_dpi_scale();
		render_text("This is the tutorial for our escape", 30, 300, 0.5f, vec4(0.7f, 0.4f, 0.1f, 0.8f), dpi_scale);
		render_text("Follow the instruction and good luck!", 30, 355, 0.5f, vec4(0.7f, 0.4f, 0.1f, 0.8f), dpi_scale);
		render_text("Please press 'n' to start the tutorial", 30, 400, 0.4f, vec4(0.5f, 0.5f, 0.5f, abs(sin(t * 2.5f))), dpi_scale);
	}


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

			}
			else {
				glUniform4fv(glGetUniformLocation(program, "diffuse"), 1, (const float*)(&g.mat->diffuse));
				glUniform1i(glGetUniformLocation(program, "use_texture"), false);
			}

			// render vertices: trigger shader programs to process vertex data
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMesh[i]->index_buffer);
			glDrawElements(GL_TRIANGLES, g.index_count, GL_UNSIGNED_INT, (GLvoid*)(g.index_start * sizeof(GLuint)));
		}
		i++;
	}
	//text render
	
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
		if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)	glfwSetWindowShouldClose(window, GL_TRUE);
		else if (key == GLFW_KEY_H || key == GLFW_KEY_F1)	print_help();
		else if (key == GLFW_KEY_HOME)					cam = camera();
		else if (key == GLFW_KEY_T)					show_texcoord = !show_texcoord;
		else if (key == GLFW_KEY_S)					scene = 1;
		else if (key == GLFW_KEY_N)					scene++;
		else if (key == GLFW_KEY_R)
		{
			b_2d = !b_2d;
		}
		else if (key == GLFW_KEY_RIGHT) {
			models[1].right_move(maps[scene]);
		}
		else if (key == GLFW_KEY_LEFT) {
			models[1].left_move(maps[scene]);
		}
		else if (key == GLFW_KEY_UP) {
			models[1].up_move(maps[scene]);
		}
		else if (key == GLFW_KEY_DOWN) {
			models[1].down_move(maps[scene]);
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

	// load the mesh
	pMesh.emplace_back(load_model(mesh_warehouse));
	pMesh.emplace_back(load_model(mesh_hero));
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

