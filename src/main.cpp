#include "cgmath.h"			// slee's simple math library
#define STB_IMAGE_IMPLEMENTATION
#include "cgut2.h"			// slee's OpenGL utility
#include "trackball.h"
#include "assimp_loader.h"
#include "model.h"
#include "map.h"
#include "wall.h"
#include "location.h"
#include "particle.h"
#include "irrKlang\irrKlang.h"
#pragma comment(lib,"irrKlang.lib")

//*******************************************************************
// forward declarations for freetype text
bool init_text();
void render_text(std::string text, GLint x, GLint y, GLfloat scale, vec4 color, GLfloat dpi_scale = 1.0f);
mat4 Ortho(float left, float right, float bottom, float top, float dnear, float dfar);
//*************************************
// global constants
static const char*	window_name = "sigMa";
static const char* mesh_warehouse = "mesh/Room/warehouse/warehouse.obj";
static const char*	mesh_hero = "mesh/Hero/robotcleaner.obj";
static const char*  wood_box = "mesh/gimmick/woodbox/woodbox.obj";
static const char* wood_doublebox = "mesh/gimmick/doublebox/doublebox.obj";
static const char* mesh_living = "mesh/Room/living/living.obj";
static const char* mesh_kitchen = "mesh/Room/kitchen/kitchen.obj";
static const char* mesh_bedroom = "mesh/Room/bed/bedroom.obj";
static const char* mesh_bathroom = "mesh/Room/bath/bathroom.obj";
static const char* mesh_flower = "mesh/Enemy/Mflower/Mflower.obj";
static const char* mesh_warehouse_key = "mesh/gimmick/key/key.obj";
static const char* mesh_living_key = "mesh/gimmick/key_black/key_black.obj";
static const char* mesh_kitchen_key = "mesh/gimmick/key_orange/key_orange.obj";
static const char* mesh_bedroom_key = "mesh/gimmick/key_red/key_red.obj";
static const char* mesh_bathroom_key = "mesh/gimmick/key_blue/key_blue.obj";

static const char* vert_shader_path = "shaders/model.vert";
static const char* frag_shader_path = "shaders/model.frag";
static const char* vert_background_path = "shaders/skybox.vert";		// text vertex shaders
static const char* frag_background_path = "shaders/skybox.frag";
static const char* vert_image = "shaders/image.vert";		// text vertex shaders
static const char* frag_image = "shaders/image.frag";
//*************************************
static const char* background_sound = "sounds/background.wav";
static const char* gameover_sound = "sounds/gameover.mp3";
static const char* gameend_sound = "sounds/gameend.mp3";
//*************************************
static const char* wall_warehouse = "texture/wall_warehouse.jpg";
static const char* wall_living = "texture/wall_living.jpg";
static const char* wall_kitchen = "texture/wall_kitchen.jpg";
static const char* wall_bedroom = "texture/wall_bedroom.jpg";
static const char* wall_bathroom = "texture/wall_bathroom.jpg";
static const char* object_door_warehouse = "texture/door.png";
static const char* object_door_living = "texture/door_black.png";
static const char* object_door_kitchen = "texture/door_yellow.png";
static const char* object_door_bedroom = "texture/door_red.png";
static const char* object_door_bathroom = "texture/door_blue.png";
static const char* beacon_warehouse = "texture/beacon_pink.png";
static const char* beacon_living = "texture/beacon_black.png";
static const char* beacon_kitchen = "texture/beacon_yellow.png";
static const char* beacon_bedroom = "texture/beacon_red.png";
static const char* beacon_bathroom = "texture/beacon_blue.png";
static const char* img_key_warehouse = "texture/key.png";
static const char* img_key_living = "texture/key_black.png";
static const char* img_key_kitchen = "texture/key_yellow.png";
static const char* img_key_bedroom = "texture/key_red.png";
static const char* img_key_bathroom = "texture/key_blue.png";
static const char* img_end = "images/ending.png";
static const char* img_checking = "texture/checking.png";
static const char* img_charge = "texture/charge.jpg";
static const char* img_start = "images/hero.png";
static const char* img_help = "images/hero_background.png";
//*************************************
static const char*	particle_explode = "particle/explode.png";
static const char*	particle_splash = "particle/splash.png";
//*************************************
irrklang::ISoundEngine* engine = nullptr;
irrklang::ISoundSource* background_src = nullptr;
irrklang::ISoundSource* gameover_src = nullptr;
irrklang::ISoundSource* gameend_src = nullptr;
//*************************************
// common structures
struct camera
{
	vec3	eye = vec3( 0, -50, 200 );
	vec3	at = vec3( 0, 0, 0 );
	vec3	up = vec3( 0, 1, 0 );
	mat4	view_matrix = mat4::look_at( eye, at, up );
		
	float	fovy = PI/4.0f; // must be in radian
	float	aspect_ratio = 0.0f;
	float	dNear = 1.0f;
	float	dFar = 1000.0f;
	mat4	projection_matrix;
};
struct light_t
{
	vec4	position = vec4(0.0f, 0.0f, 50.0f, 1.0f);   // spot light
	vec4	position_2d = vec4(100.0f, 0.0f, 0.0f, 0.0f);
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

struct state
{
	int	  scene;
	map_t save_map;
	std::vector<model_t> save_model;
	std::vector<wall_t> save_wall;
	float save_time_passed;
	float save_charge;
	int	  pat1;

};

struct enemy_state
{
	float search_interval = 2.0f;
	float latest_search = 0.0f;
};

struct herostate
{
	float energy;
	float total_time;
	float decrease_rate;
	float left_time;
	float left_energy;

	float stopped = 0.0f;
	float passed = 0.0f;
	float save_passed = 0.0f;
	float total_charging = 0.0f;
	float save_charging = 0.0f;

	herostate() { energy = 50.0f; total_time = 100.0f; decrease_rate = 1.0f; left_time = 100.0f; left_energy = 50.0f; }
	herostate(float e, float t) { energy = e; total_time = t; decrease_rate = 1.0f; left_time = t; left_energy = e;}
	
};

//*************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = cg_default_window_size(); // initial window size

//*************************************
// OpenGL objects
	
GLuint	wall_vertex_array = 0;
GLuint	WALL_warehouse = 0;
GLuint	WALL_living = 0;
GLuint	WALL_kitchen = 0;
GLuint	WALL_bedroom = 0;
GLuint	WALL_bathroom = 0;

GLuint	DOOR_warehouse = 0;
GLuint	DOOR_living = 0;
GLuint	DOOR_kitchen = 0;
GLuint	DOOR_bedroom = 0;
GLuint	DOOR_bathroom = 0;

GLuint	BEACON_warehouse = 0;
GLuint	BEACON_living = 0;
GLuint	BEACON_kitchen = 0;
GLuint	BEACON_bedroom = 0;
GLuint	BEACON_bathroom = 0;

GLuint	KEY_warehouse = 0;
GLuint	KEY_living = 0;
GLuint	KEY_kitchen = 0;
GLuint	KEY_bedroom = 0;
GLuint	KEY_bathroom = 0;

GLuint  CHARGE = 0;
GLuint	CHECKING = 0;

GLuint	START = 0;
GLuint	HELP = 0;
GLuint	END = 0;

GLuint		VAO_IMAGE;
GLuint		VAO_BACKGROUND;			// vertex array for text objects

GLuint		program = 0;		// ID holder for GPU program
GLuint		program_background = 0;	// GPU program for text render
GLuint		program_img = 0;
GLuint		program_shadow = 0;

GLuint		PARTICLE1 = 0;
GLuint		PARTICLE2 = 0;
//*************************************
// global variables
int		frame = 0;		// index of rendering frames
bool	b_2d = false;
bool	b_help = false;
bool	pause = false;
bool	b_game = false;
bool	b_kill = false;
bool	b_sound = false;
bool	in_game = false;
bool	now_charge = false;
bool	b_clear = false;
float	t;
float	dead_interval = 1.5f;
float	t_game;
float	t_kill;
float	start_t;
float	start_charge;
int		is_exec = 0;
int		help_count = 0;
int		pat1_count = 0;

auto	models = std::move(set_pos()); // positions of models
auto	maps = std::move(create_grid());
auto	walls = std::move(set_wall());
int		scene = 0;
int		difficulty = 0;
int		cur_tex = 0;
GLuint	wall_tex[5];

map_t	cur_map;
int		keys[6];
state	save_states[10];
state	key_state;
int		key_scene = 0;
int		killed_index = 0;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

std::vector<std::string> skyboxes = { "skybox/front.jpg", "skybox/back.jpg", "skybox/right.jpg", "skybox/left.jpg", "skybox/top.jpg", "skybox/bottom.jpg"};
GLuint skyboxTexture;
model_t* hero;

//*************************************
// holder of vertices and indices of a unit wall
std::vector<vertex>	unit_wall_vertices;
std::vector<particle_t> particles;
//*************************************
// scene objects
std::vector<mesh2*>		pMesh;
camera		cam;
float		cam_xpos = 200.0f;
float		cam_xmax = 315.0f;
trackball	tb;
light_t		light;
material_t	materials;
herostate	hero_state;
enemy_state flower;
enemy_state flower2 = { 0.5f, 0.0f };


#pragma region GAME_MANAGE  //Game over check

//if the return value is 1, game over
int game_over_chk() {
	vec2 hero_pos = vec2(hero->cur_pos.x, hero->cur_pos.y);
	
	if (in_game) {
		if (b_2d) {
			for (int i = int(cur_map.grid.x) - 1; i > -1; i--) {
				if ((int)hero_pos.x == i) break;
				else if (cur_map.map[i][(int)hero_pos.y] != CANMOVE) {
					return 1;
				}
			}
		}

		for (int i = (int)hero->cur_pos.x - 1; i < (int)hero->cur_pos.x + 2; i++) {
			
			if (i < 0 || i >(int)cur_map.grid.x - 1) continue;
			for (int j = (int)hero->cur_pos.y - 1; j < (int)hero->cur_pos.y + 2; j++) {
				if (j < 0 || j >(int)cur_map.grid.y - 1) continue;
				
				if (cur_map.map[i][j] == 4 || cur_map.map[i][j] == 11 || cur_map.map[i][j] == 15) return 1;
			}
		}

		if (hero_state.left_energy < 0.0f || hero_state.left_time < 0.0f) return 1;
	}
	return 0 ;
}

void game_over() {
	if (is_exec) return;
	is_exec = 1;

	killed_index = 1;

	b_2d = true;
	pause = true;
	b_game = true;
	t_game = float(glfwGetTime());
	hero->active = false;
	particles.clear();

	for (int p = 0; p < particle_t::MAX_PARTICLES; p++) {
		particles.emplace_back(particle_t::particle_t(hero->center, t_game, 1));
	}

	if (engine->isCurrentlyPlaying(background_src)) {
		engine->stopAllSounds();
	}
}

void door_active_chk() {
	for (int i = 6; i < 11; i++) {
		if (walls[i].active) {
			if (walls[i].pos.x > -1) {
				if (cur_map.map[(int)walls[i].pos.x][(int)walls[i].pos.y] != 0) walls[walls[i].id].active = true;
				else walls[walls[i].id].active = false;
			}
			else if (walls[i].direction == 0) {
				if (walls[i].wallpos_z == 1) {
					if (cur_map.map[0][walls[i].wallpos] == 10) walls[walls[i].id].active = true;
					else walls[walls[i].id].active = false;
					return;
				}

				if (cur_map.map[0][walls[i].wallpos] != 0) walls[walls[i].id].active = true;
				else walls[walls[i].id].active = false;
			}
			else if (walls[i].direction == 1) {
				if (walls[i].wallpos_z == 1) {
					if (cur_map.map[walls[i].wallpos][(int)cur_map.grid.y - 1] == 10) walls[walls[i].id].active = true;
					else walls[walls[i].id].active = false;
					return;
				}

				if (cur_map.map[walls[i].wallpos][(int)cur_map.grid.y - 1] != 0) walls[walls[i].id].active = true;
				else walls[walls[i].id].active = false;
			}
		}
	}
}

void enemy_killed(model_t& enemy) {
	if (b_2d) {
		if (enemy.active) {
			for (int i = int(cur_map.grid.x) - 1; i > -1; i--) {
				if ((int)enemy.cur_pos.x == i) break;
				else if (cur_map.map[i][(int)enemy.cur_pos.y] != CANMOVE) {
					
					if (is_exec) return;
					killed_index = enemy.index;
					b_kill = true;
					//is_exec = 1;
					enemy.active = false;
					cur_map.map[(int)enemy.cur_pos.x][(int)enemy.cur_pos.y] = 0;

					t_kill = float(glfwGetTime());
					particles.clear();

					for (int p = 0; p < particle_t::MAX_PARTICLES; p++) {
						particles.emplace_back(particle_t::particle_t(enemy.center, t_kill, 0));
					}
		
					if (difficulty == 1) {
						if (killed_index == 4) {
							models[6].active = true;
							obj_3d_pos(models[6], cur_map, 7, vec2(models[killed_index].cur_pos.x, models[killed_index].cur_pos.y));
						}
					}
				}
			}
		}
	}

}


void enemy_search(model_t& enemy) {
	if (enemy.active && !pause) {
		if (t - flower.latest_search > flower.search_interval) {
			if (enemy.cur_pos.x - hero->cur_pos.x > 0.0f && enemy.cur_pos.x - hero->cur_pos.x <= 4.0f) { // left
				enemy.theta = 0;
				enemy.left_move(cur_map, models, walls, keys);
			}
			else if (enemy.cur_pos.x - hero->cur_pos.x < 0.0f && enemy.cur_pos.x - hero->cur_pos.x >= -4.0f) { //right
				enemy.theta = PI;
				enemy.right_move(cur_map, models, walls, keys);
			}
			if (enemy.cur_pos.y - hero->cur_pos.y < 0.0f && enemy.cur_pos.y - hero->cur_pos.y >= -4.0f) { // up
				enemy.theta = -PI / 2;
				enemy.up_move(cur_map, models, walls, keys);
			}
			else if (enemy.cur_pos.y - hero->cur_pos.y > 0.0f && enemy.cur_pos.y - hero->cur_pos.y <= 4.0f) { // down
				enemy.theta = PI / 2;
				enemy.down_move(cur_map, models, walls, keys);
			}
			flower.latest_search = t;
		}
	}
}

void enemy_move_pat1(model_t& enemy) {
	if (enemy.active && !pause) {
		if (t - flower2.latest_search > flower2.search_interval) {
			printf("test2");
			if (pat1_count < 2) {
				enemy.down_move(cur_map, models, walls, keys);
				enemy.theta = PI / 2;
			}
			else if (pat1_count <= 4) {
				enemy.right_move(cur_map, models, walls, keys);
				enemy.theta = PI;
			}
			else if (pat1_count <= 7) {
				enemy.left_move(cur_map, models, walls, keys);
				enemy.theta = 0;
			}
			else if (pat1_count <= 9) {
				enemy.up_move(cur_map, models, walls, keys);
				enemy.theta = -PI / 2;
			}
			pat1_count++;

			if (pat1_count == 10) pat1_count = 0;
			
			flower2.latest_search = t;
		}
	}
}

void key_capture() {
	key_scene = scene;

	key_state.save_map = cur_map;

	hero_state.save_passed = hero_state.passed;
	key_state.save_charge = hero_state.save_charging;
	start_t = float(glfwGetTime());

	key_state.save_model = models;
	key_state.save_wall = walls;
	key_state.pat1 = pat1_count;
}

void key_active_chk(wall_t& key, int key_scene) {
	//key active
	if (keys[key.id - 4] != 1 && key_scene == scene) {
		if (b_2d) {
			key.active = false;
			
			if (cur_map.map[(int)hero->cur_pos.x][key.wallpos] == 1) {
				keys[key.id - 4] = 1;
				key_capture();
			}

			if (cur_map.map[(int)hero->cur_pos.x][key.wallpos] == 0) {
				models[key.id].active = true;
				obj_3d_pos(models[key.id], cur_map, scene, vec2(hero->cur_pos.x, (float)key.wallpos));
			}
		}
		else {
			key.active = true;
			models[key.id].active = false;
			if (cur_map.map[(int)models[key.id].cur_pos.x][(int)models[key.id].cur_pos.y] == models[key.id].index)
				cur_map.map[(int)models[key.id].cur_pos.x][(int)models[key.id].cur_pos.y] = 0;
		}
	}
}

void rules_level(int level) {

	if (is_exec) return;

	door_active_chk();

	switch (level) {
	case 1:
		enemy_killed(models[4]);
		enemy_killed(models[11]);
		break;
	case 2:
		enemy_killed(models[4]);
		enemy_killed(models[11]);
		enemy_killed(models[15]);
		enemy_search(models[4]);
		enemy_move_pat1(models[15]);
		key_active_chk(walls[12], 6);

		if (scene == 9 && keys[2] != 1) {
			for (int i = 0; i < (int)cur_map.grid.x; i++) {
				if (cur_map.map[i][6] == 2) {
					for (int j = 0; j < (int)cur_map.grid.x; j++) {
						if (cur_map.map[j][7] == 10) {
							models[6].active = true;
							obj_3d_pos(models[6], cur_map, 9, vec2(6, 0));
							break;
						}
						else {
							models[6].active = false;
						}
					}
				}
			}
		}

		break;
	case 3:
		enemy_killed(models[4]);
		enemy_search(models[4]);
		key_active_chk(walls[12], 6);
		break;
	}
	
}

void restart() {
	b_2d = false;
	b_game = false;
	in_game = true;
	b_sound = false;
	is_exec = 0;
	pause = false;
	hero->active = true;
	engine->stopAllSounds();
	engine->play2D(background_src, true);
}

void charging() {
	
	if (walls[11].active) {
		if (cur_map.map[int(walls[11].pos.x)][int(walls[11].pos.y)] == 1) {
			if (!now_charge) {
				start_charge = float(glfwGetTime());
				now_charge = !now_charge;
			}
			hero_state.total_charging = hero_state.save_charging + (t - start_charge) * 3;
			if (hero_state.left_energy + hero_state.total_charging > hero_state.energy) hero_state.total_charging = hero_state.energy - hero_state.left_energy;
			
		}
		else {
			hero_state.save_charging = hero_state.total_charging;
			now_charge = false;
		} 
	}
	else now_charge = false;

	hero_state.left_energy += hero_state.total_charging;
}

void calcEnergy() {
	if (!pause) {
		hero_state.passed = hero_state.save_passed + t - start_t - hero_state.stopped;
	}
	else {
		hero_state.stopped = hero_state.save_passed + t - start_t - hero_state.passed;
	}
	hero_state.left_energy = hero_state.energy - hero_state.passed * hero_state.decrease_rate;
	
	return;
}

void calcTime(){
	if (!pause) {
		hero_state.passed = hero_state.save_passed + t - start_t - hero_state.stopped;
	}
	else {
		hero_state.stopped = hero_state.save_passed + t - start_t - hero_state.passed;
	}
	hero_state.left_time = hero_state.total_time - hero_state.passed * hero_state.decrease_rate;
}


std::string EnergyBar() {
	std::string s;
	
	int left = int(hero_state.left_energy / (hero_state.energy / 10));

	if (hero_state.left_energy < 0.0f) return "**********";

	for (int i = 0; i < 10; i++) {
		if (i <= left) {
			s += "O";
		}
		else {
			s += "*";
		}
	}
	return s;
}

std::string LeftTime() {
	
	return std::to_string(hero_state.left_time) + "s";
}
GLuint loadCubemap(std::vector<std::string> faces) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(absolute_path(faces[i].c_str()), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
#pragma endregion

#pragma region Scenes

void load_start_scene(int scene) {
	
	float dpi_scale = cg_get_dpi_scale();
	

	switch (scene) {
	case 0:
		glUseProgram(program_img);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, START);
		glUniform1i(glGetUniformLocation(program_img, "TEX"), 3);
		glBindVertexArray(VAO_IMAGE);
		// render quad vertices
		glDrawArrays(GL_TRIANGLES, 0, 6);
		render_text("DOM_HWANG_CHA", 50, 100, 1.0f, vec4(0.5f, 0.8f, 0.2f, 1.0f), dpi_scale);
		render_text("Team sigma - Dongmin, Dongjun, Jiye", 50, 355, 0.5f, vec4(0.7f, 0.4f, 0.1f, 0.8f), dpi_scale);
		render_text("Please 's' to start", 50, 400, 0.6f, vec4(0.5f, 0.5f, 0.5f, abs(sin(t * 2.5f))), dpi_scale);
		break;
	case 1:
		glUseProgram(program_img);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, HELP);
		glUniform1i(glGetUniformLocation(program_img, "TEX"), 3);
		glBindVertexArray(VAO_IMAGE);
		// render quad vertices
		glDrawArrays(GL_TRIANGLES, 0, 6);
		render_text("My name is zetbot.. I'm vending machine..", 30, 355, 0.5f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("Please press 'n' to next", 30, 400, 0.4f, vec4(1.0f, 1.0f, 1.0f, abs(sin(t * 2.5f))), dpi_scale);
		
		break;
	case 2:
		glUseProgram(program_img);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, HELP);
		glUniform1i(glGetUniformLocation(program_img, "TEX"), 3);
		glBindVertexArray(VAO_IMAGE);
		// render quad vertices
		glDrawArrays(GL_TRIANGLES, 0, 6);
		render_text("I can't live in this house cleaning anymore!", 30, 300, 0.5f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("I'm going to escape!", 30, 355, 0.5f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("Please press 'n' to next", 30, 400, 0.4f, vec4(1.0f, 1.0f, 1.0f, abs(sin(t * 2.5f))), dpi_scale);
		break;
	case 3:
		glUseProgram(program_img);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, HELP);
		glUniform1i(glGetUniformLocation(program_img, "TEX"), 3);
		glBindVertexArray(VAO_IMAGE);
		// render quad vertices
		glDrawArrays(GL_TRIANGLES, 0, 6);
		render_text("To escape, move the box in 3D and get the key", 30, 300, 0.5f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("...... ", 30, 355, 0.5f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("Please press 'n' to next", 30, 400, 0.4f, vec4(1.0f, 1.0f, 1.0f, abs(sin(t * 2.5f))), dpi_scale);
		break;
	case 4:
		glUseProgram(program_img);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, HELP);
		glUniform1i(glGetUniformLocation(program_img, "TEX"), 3);
		glBindVertexArray(VAO_IMAGE);
		// render quad vertices
		glDrawArrays(GL_TRIANGLES, 0, 6);
		render_text("I have to escape the room within a certain time.", 30, 300, 0.5f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("IF my battery runs out or the owner retuns, I fail.. ", 30, 355, 0.5f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("Please press 'n' to next", 30, 400, 0.4f, vec4(1.0f, 1.0f, 1.0f, abs(sin(t * 2.5f))), dpi_scale);
		break;
	case 5:
		glUseProgram(program_img);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, HELP);
		glUniform1i(glGetUniformLocation(program_img, "TEX"), 3);
		glBindVertexArray(VAO_IMAGE);
		// render quad vertices
		glDrawArrays(GL_TRIANGLES, 0, 6);
		render_text("Please choose the difficulty of the game", 30, 100, 0.5f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("#1 Easy - Number of room : 2", 30, 150, 0.3f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("#2 Middle - Number of room : 4", 30, 200, 0.3f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("#3 Hard - Number of room : 5", 30, 250, 0.3f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("Follow the instruction and good luck!", 30, 355, 0.5f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("Please press number of the difficulty to start", 30, 400, 0.4f, vec4(1.0f, 1.0f, 1.0f, abs(sin(t * 2.5f))), dpi_scale);
		break;
	
	default:
		break;
	}
	
	return;
}

void set_false() {
	int i = 0;
	for (auto& m : models) {
		if (i != 0 && i != 1) m.active = false;
		i++;
	}
	i = 0;
	for (auto& w : walls) {
		if (i != 0) w.active = false;
		i++;
	}
	return;
}

void load_help_scene() {
	
	b_2d = false;
	glUseProgram(program_img);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, HELP);
	glUniform1i(glGetUniformLocation(program_img, "TEX"), 3);
	glBindVertexArray(VAO_IMAGE);
	// render quad vertices
	glDrawArrays(GL_TRIANGLES, 0, 6);
	float dpi_scale = cg_get_dpi_scale();
	render_text("Instruction", 200, 60, 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
	render_text("Room escape game that goes beyond 2D and 3D.", 100, 100, 0.5f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);

	if (help_count == 1) {
		render_text("You have to escape within the limited time.", 30, 200, 0.4f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("You can move using up, down, left, right keys.", 30, 250, 0.4f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("Press 'F' button to change the dimension.", 30, 300, 0.4f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("Press 'A' button to pull the object.", 30, 350, 0.4f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("Press 'S' button to push the object.", 30, 400, 0.4f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("Only one object can be pushed/pulled at a time.", 30, 450, 0.4f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
	}
	else if (help_count == 2) {
		render_text("You can reset the game by using 'R' button.", 30, 200, 0.4f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("Something special happens when you obscure a 2d object.", 30, 250, 0.4f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("When in 2d mode, it will die if it is obscured by an object.", 30, 300, 0.4f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("Enemies will also die if they are obscured by objects in 2d mode.", 30, 350, 0.4f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("It dies when it runs out of energy.", 30, 400, 0.4f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("Energy can be recharged at a charging station.", 30, 450, 0.4f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
	}
	else if (help_count == 3) {
		render_text("If you acquire a key, your progress will be saved.", 30, 200, 0.4f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
		render_text("Go through the door and go to the next room.", 30, 250, 0.4f, vec4(1.0f, 1.0f, 1.0f, 0.8f), dpi_scale);
	}
}

void load_game_scene(int scene) {
	switch (scene) {
	case 6:
		b_2d = false;

		//set camera
		cam_xpos = 200.0f;
		cam_xmax = 315.0f;
		
		//set warehouse
		models[0].id = 0; 
		//cur_map = maps[0];
		cur_tex = 0;

		//set wall
		walls[0].center = vec3(-39.49f, 0.0f, 26.0f);
		walls[0].size = vec2(154.0f, 50.0f);
		
		break;
	case 7:
		b_2d = false;

		//set camera
		cam_xpos = 275.0f;
		cam_xmax = 400.0f;

		//set warehouse
		models[0].id = 3;
		//cur_map = maps[1];
		cur_tex = 1;

		//set wall
		walls[0].center = vec3(-114.49f, 0.0f, 26.0f);
		walls[0].size = vec2(229.0f, 50.0f);

		break;
	case 8:
		b_2d = false;

		//set camera
		cam_xpos = 200.0f;
		cam_xmax = 315.0f;

		//set warehouse
		models[0].id = 4;
		//cur_map = maps[2];
		cur_tex = 2;

		//set wall
		walls[0].center = vec3(-76.99f, 0.0f, 26.0f);
		walls[0].size = vec2(79.0f, 50.0f);
		break;
	case 9:
		b_2d = false;

		//set camera
		cam_xpos = 237.5f;
		cam_xmax = 330.0f;

		//set warehouse
		models[0].id = 5;
		//cur_map = maps[3];
		cur_tex = 3;

		//set wall
		walls[0].center = vec3(-76.99f, 0.0f, 26.0f);
		walls[0].size = vec2(154.0f, 50.0f);

		break;
	case 10:
		b_2d = false;

		//set camera
		cam_xpos = 237.5f;
		cam_xmax = 330.0f;
		//set warehouse
		models[0].id = 6;
		//cur_map = maps[4];
		cur_tex = 4;

		//set wall
		walls[0].center = vec3(-76.99f, 0.0f, 26.0f);
		walls[0].size = vec2(79.0f, 50.0f);

		break;
	default:
		break;
	}
}

void capture(int scene) {
	save_states[scene].save_map = cur_map;

	save_states[scene].save_model = models;
	save_states[scene].save_wall = walls;
}

void load_state(int load_scene) {

	/*
	//set all object false
	set_false();
	*/

	//map loading
	cur_map = save_states[load_scene].save_map;

	//load model setting
	models = save_states[load_scene].save_model;
	walls = save_states[load_scene].save_wall;
}

void reset() {
	//change key_scene
	scene = key_scene;
	start_t = float(glfwGetTime());

	//map loading
	cur_map = key_state.save_map;

	hero_state.stopped = 0;
	hero_state.save_charging = key_state.save_charge;
	hero_state.total_charging = hero_state.save_charging;

	//load model setting
	models = key_state.save_model;
	walls = key_state.save_wall;
	pat1_count = key_state.pat1;

	load_game_scene(key_scene);
	
}

void init_state(int level) {

	//key setting
	for (int i = 0; i < 6; i++) keys[i] = 0;

	switch (level) {
	case 1:

		//time set
		start_t = float(glfwGetTime());
		hero_state = herostate(25.0f, 60.0f);

		//-----------state scene 6-------------------
		
		cur_map = maps[0];
		
		//set all object false
		set_false();

		walls[6].active = true;
		walls[11].active = true;

		//set beacon
		obj_floor_pos(walls[6], 6, vec2(3, 9));

		//set charge
		obj_floor_pos(walls[11], 6, vec2(4, 9));

		//active obj
		models[2].active = true;
		models[3].active = true;
		models[5].active = true;

		//set hero pos
		obj_3d_pos(*hero, cur_map, 6, vec2(4, 9));

		//set wood pos
		obj_3d_pos(models[2], cur_map, 6, vec2(1, 5));
		obj_3d_pos(models[3], cur_map, 6, vec2(2, 2));

		//set door pos
		obj_2d_pos(walls[1], 6, 0, 8, 0, vec2(1, 2));

		//set key pos
		obj_3d_pos(models[5], cur_map, 6, vec2(0, 0));
		capture(6);

		//-----------state scene 7-----------------
		
		cur_map = maps[1];

		//set all object false
		set_false();

		walls[7].active = true;
		walls[11].active = true;

		//set beacon
		obj_floor_pos(walls[7], 7, vec2(1, 13));

		//set charge
		obj_floor_pos(walls[11], 7, vec2(13, 7));

		//active obj
		models[2].active = true;
		models[3].active = true;

		models[4].active = true;
		models[11].active = true;

		//set hero pos
		obj_3d_pos(*hero, cur_map, 7, vec2(0, 1));

		//set wood pos
		obj_3d_pos(models[2], cur_map, 7, vec2(5, 10));
		obj_3d_pos(models[3], cur_map, 7, vec2(11, 4));

		//set enemy pos
		models[4].theta = PI / 2;
		obj_3d_pos(models[4], cur_map, 7, vec2(8, 11));
		models[11].theta = 0;
		obj_3d_pos(models[11], cur_map, 7, vec2(6, 3));

		//set door pos
		obj_2d_pos(walls[1], 7, 0, 2, 0, vec2(1, 2));
		obj_2d_pos(walls[2], 7, 1, 8, 0, vec2(1, 2));

		//set key pos
		//obj_3d_pos(models[6], cur_map, 7, vec2(12, 7));

		capture(7);

		break;
	case 2:

		//time set
		start_t = float(glfwGetTime());
		hero_state = herostate(25.0f, 100.0f);

		//-----------state scene 6-------------------

		cur_map = maps[0];

		//set all object false
		set_false();

		walls[6].active = true;
		walls[11].active = true;
		walls[12].active = true;

		//set beacon
		obj_floor_pos(walls[6], 6, vec2(1, 0));

		//set charge
		obj_floor_pos(walls[11], 6, vec2(0, 7));

		//active obj
		models[2].active = true;
		models[3].active = true;
		models[5].active = false;

		//set hero pos
		obj_3d_pos(*hero, cur_map, 6, vec2(0, 7));

		//set wood pos
		obj_3d_pos(models[3], cur_map, 6, vec2(1, 2));
		obj_3d_pos(models[2], cur_map, 6, vec2(3, 9));

		//set door pos
		obj_2d_pos(walls[1], 6, 1, 3, 0, vec2(1, 2));

		//set key pos
		obj_2d_pos(walls[12], 6, 0, 4, 0, vec2(1, 2));
		capture(6);

		//-----------state scene 7-----------------

		cur_map = maps[1];

		//set all object false
		set_false();
		walls[6].active = true;
		walls[7].active = true;
		walls[9].active = true;
		walls[11].active = true;

		//set beacon
		obj_floor_pos(walls[6], 7, vec2(4, 2));
		obj_floor_pos(walls[7], 7, vec2(10, 2));
		obj_2d_pos(walls[9], 7, 1, 2, 1, vec2(1, 1));

		//set charge
		obj_floor_pos(walls[11], 7, vec2(13, 7));

		//active obj
		models[8].active = true;

		models[2].active = true;
		models[3].active = true;
		models[10].active = true;
		models[13].active = true;

		models[4].active = true;
		models[11].active = true;
		models[15].active = true;


		//set hero pos
		obj_3d_pos(*hero, cur_map, 7, vec2(0, 1));

		//set wood pos
		obj_3d_pos(models[2], cur_map, 7, vec2(1, 3));
		obj_3d_pos(models[10], cur_map, 7, vec2(4, 10));
		obj_3d_pos(models[3], cur_map, 7, vec2(11, 9));
		obj_3d_pos(models[13], cur_map, 7, vec2(12, 9));


		//set enemy pos
		models[4].theta = 0;
		obj_3d_pos(models[4], cur_map, 7, vec2(5, 3));
		models[11].theta = PI / 2;
		obj_3d_pos(models[11], cur_map, 7, vec2(9, 14));
		models[15].theta = PI / 2;
		obj_3d_pos(models[15], cur_map, 7, vec2(11, 14));

		//set door pos
		obj_2d_pos(walls[1], 7, 0, 2, 0, vec2(1, 2));
		obj_2d_pos(walls[2], 7, 1, 8, 0, vec2(1, 2));
		obj_2d_pos(walls[4], 7, 0, 10, 0, vec2(1, 2));

		//set key pos
		obj_3d_pos(models[8], cur_map, 7, vec2(14, 13));

		capture(7);

		//-----------state scene 9-------------------

		cur_map = maps[3];

		//set all object false
		set_false();

		walls[9].active = true;
		walls[11].active = true;

		walls[17].active = true;
		walls[18].active = true;
		walls[19].active = true;

		//set beacon
		obj_floor_pos(walls[9], 9, vec2(8, 3));

		//set charge
		obj_floor_pos(walls[11], 9, vec2(1, 2));

		//set checking
		obj_2d_pos(walls[17], 9, 0, 6, 0, vec2(1, 1));
		obj_2d_pos(walls[18], 9, 0, 7, 1, vec2(1, 1));
		obj_2d_pos(walls[19], 9, 0, 7, 0, vec2(1, 1));

		//active obj
		models[2].active = true;
		models[10].active = true;
		models[5].active = false;

		//set hero pos
		obj_3d_pos(*hero, cur_map, 9, vec2(4, 9));

		//set wood pos
		obj_3d_pos(models[10], cur_map, 9, vec2(6, 4));
		obj_3d_pos(models[2], cur_map, 9, vec2(6, 5));

		//set door pos
		obj_2d_pos(walls[4], 9, 1, 4, 0, vec2(1, 2));

		capture(9);

		break;
	case 3:

		//time set
		start_t = float(glfwGetTime());
		hero_state = herostate(25.0f, 120.0f);

		//-----------state scene 6-------------------

		cur_map = maps[0];

		//set all object false
		set_false();

		walls[6].active = true;
		walls[11].active = true;
		walls[12].active = true;

		//set beacon
		obj_2d_pos(walls[6], 6, 1, 2, 1, vec2(1, 1));

		//set charge
		obj_floor_pos(walls[11], 6, vec2(0, 0));

		//active obj
		models[10].active = true;
		models[3].active = true;
		models[5].active = false;

		//set hero pos
		obj_3d_pos(*hero, cur_map, 6, vec2(2, 3));

		//set wood pos
		obj_3d_pos(models[3], cur_map, 6, vec2(1, 7));
		obj_3d_pos(models[10], cur_map, 6, vec2(1, 6));

		//set door pos
		obj_2d_pos(walls[1], 6, 0, 8, 0, vec2(1, 2));

		//set key pos
		obj_2d_pos(walls[12], 6, 0, 4, 0, vec2(1, 2));
		capture(6);
		break;
	}

}

void load_level(int level) {

	restart();

	init_state(level);

	//load scene 6
	scene = 6;
	load_state(scene);
	load_game_scene(scene);

	//save cur_state == init_state
	key_capture();

	in_game = true;
	
}
#pragma endregion


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
std::vector<vertex> create_wall() // important
{
	std::vector<vertex> v; // origin
	float norm = sqrt(2.0f);
	v.push_back({ vec3(0, 0.5f,-0.5f), vec3(0, 1 / norm,-1.0f / norm), vec2(0,0) });
	v.push_back({ vec3(0, -0.5f,-0.5f), vec3(0, -1.0f / norm,-1.0f / norm), vec2(1,0) });
	v.push_back({ vec3(0, -0.5f, 0.5f), vec3(0, -1.0f / norm, 1 / norm), vec2(1,1) });
	v.push_back({ vec3(0, 0.5f, 0.5f), vec3(0, 1 / norm, 1 / norm), vec2(0,1) });

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
	calcTime();
	calcEnergy();
	charging();
	
	if (game_over_chk()) game_over();
	rules_level(difficulty);
	

	glUseProgram(program);

	// update projection matrix
	cam.aspect_ratio = window_size.x/float(window_size.y);
	if (b_2d) { // ī�޶� �κ�
		mat4 aspect_matrix =
		{
			std::min(1 / cam.aspect_ratio,1.0f), 0, 0, 0,
			0, std::min(cam.aspect_ratio,1.0f), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};
		cam.projection_matrix = aspect_matrix * Ortho(-30.f, 30.f, -10.0f, 40.0f, 160.5f, cam_xmax); 
		cam.view_matrix = mat4::look_at(vec3(cam_xpos, models[1].center.y, 10), vec3(0, models[1].center.y, 10), vec3( -1, 0, 1 )); 
		glUniform4fv(glGetUniformLocation(program, "light_position"), 1, light.position_2d);
	}
	else {
		cam.view_matrix = mat4::look_at(models[1].center+vec3(0, -30, 140), models[1].center, vec3(0, 1, 0));
		cam.projection_matrix = mat4::perspective(cam.fovy, cam.aspect_ratio, cam.dNear, cam.dFar);
		glUniform4fv(glGetUniformLocation(program, "light_position"), 1, light.position);
	}

	// build the model matrix for oscillating scale
	t = float(glfwGetTime());
	
	// update uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation( program, "view_matrix" );			if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, cam.view_matrix );
	uloc = glGetUniformLocation( program, "projection_matrix" );	if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, cam.projection_matrix );
	
	// setup light properties
	glUniform4fv(glGetUniformLocation(program, "Ia"), 1, light.ambient);
	glUniform4fv(glGetUniformLocation(program, "Id"), 1, light.diffuse);
	glUniform4fv(glGetUniformLocation(program, "Is"), 1, light.specular);

	// setup material properties
	glUniform4fv(glGetUniformLocation(program, "Ka"), 1, materials.ambient);
	glUniform4fv(glGetUniformLocation(program, "Kd"), 1, materials.diffuse);
	glUniform4fv(glGetUniformLocation(program, "Ks"), 1, materials.specular);
	glUniform1f(glGetUniformLocation(program, "shininess"), materials.shininess);
}

#pragma region Render

void render_init() {
	// clear screen (with background color) and clear depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthMask(GL_FALSE);
	glActiveTexture(GL_TEXTURE0);
	glUseProgram(program_background);
	GLint uloc1;
	uloc1 = glGetUniformLocation(program_background, "view_matrix");			if (uloc1 > -1) glUniformMatrix4fv(uloc1, 1, GL_TRUE, cam.view_matrix);
	uloc1 = glGetUniformLocation(program_background, "projection_matrix");	if (uloc1 > -1) glUniformMatrix4fv(uloc1, 1, GL_TRUE, mat4::perspective(cam.fovy, cam.aspect_ratio, cam.dNear, cam.dFar));
	glBindVertexArray(VAO_BACKGROUND);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);

}

void render()
{
	render_init();

	if (b_clear) { // end phase
		if (engine->isCurrentlyPlaying(background_src)) {
			engine->stopAllSounds();
			engine->play2D(gameend_src, true);

		}
		b_2d = false;
		glUseProgram(program_img);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, END);
		glUniform1i(glGetUniformLocation(program_img, "TEX"), 0);
		glBindVertexArray(VAO_IMAGE);
		// render quad vertices
		glDrawArrays(GL_TRIANGLES, 0, 6);
		float dpi_scale = cg_get_dpi_scale();
		render_text("GAME CLEAR!", window_size.x / 2 - 150, 70, 1.5f, vec4(0.5f, 0.5f, 0.1f, 0.8f), dpi_scale);
		render_text("Please press 'ESC' to finish the game", window_size.x / 2 - 130, 400, 0.4f, vec4(1.0f, 1.0f, 1.0f, abs(sin(t * 2.5f))), dpi_scale);
		goto skip;
	}

	// notify GL that we use our own program
	if (b_game && ( t - t_game > dead_interval)) {
		if (!engine->isCurrentlyPlaying(gameover_src) && !b_sound) {
			engine->play2D(gameover_src, false);
			b_sound = true;
		}
		if (killed_index == 1) {
			float dpi_scale = cg_get_dpi_scale();
			render_text("GAME OVER!", window_size.x / 2 - 150, 70, 1.5f, vec4(0.7f, 0.1f, 0.1f, 0.8f), dpi_scale);
			render_text("Please press 'R' to restart stage!", window_size.x / 2 - 150, 400, 0.4f, vec4(1.0f, 1.0f, 1.0f, abs(sin(t * 2.5f))), dpi_scale);
			goto skip;
		}
		
		
	}

	// scene < 6 (game story)
	load_start_scene(scene);
	if (b_help == true) {
		load_help_scene();
	}
	glUseProgram(program);
	if (scene > 5) {
		glUseProgram(program);
		int i = 0;
		glActiveTexture(GL_TEXTURE0);
		// bind vertex array object
		for (auto& m : models) {
			if (!m.active) continue;
			
			glBindVertexArray(pMesh[m.id]->vertex_array);
			m.update(t);
			GLint uloc;
			uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, m.model_matrix);
			for (size_t k = 0, kn = pMesh[m.id]->geometry_list.size(); k < kn; k++) {
				geometry& g = pMesh[m.id]->geometry_list[k];


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
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMesh[m.id]->index_buffer);
				glDrawElements(GL_TRIANGLES, g.index_count, GL_UNSIGNED_INT, (GLvoid*)(g.index_start * sizeof(GLuint)));
			}
			i++;
		}
		glBindVertexArray(wall_vertex_array);
		glActiveTexture(GL_TEXTURE1);								// select the texture slot to bind
		glBindTexture(GL_TEXTURE_2D, wall_tex[cur_tex]);
		glActiveTexture(GL_TEXTURE2);								// select the texture slot to bind
		glBindTexture(GL_TEXTURE_2D, DOOR_warehouse);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, DOOR_living);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, DOOR_kitchen);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, DOOR_bedroom);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, DOOR_bathroom);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, BEACON_warehouse);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, BEACON_living);
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, BEACON_kitchen);
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, BEACON_bedroom);
		glActiveTexture(GL_TEXTURE11);
		glBindTexture(GL_TEXTURE_2D, BEACON_bathroom);
		glActiveTexture(GL_TEXTURE12);
		glBindTexture(GL_TEXTURE_2D, CHARGE);
		glActiveTexture(GL_TEXTURE13);
		glBindTexture(GL_TEXTURE_2D, KEY_warehouse);
		glActiveTexture(GL_TEXTURE14);
		glBindTexture(GL_TEXTURE_2D, KEY_living);
		glActiveTexture(GL_TEXTURE15);
		glBindTexture(GL_TEXTURE_2D, KEY_kitchen);
		glActiveTexture(GL_TEXTURE16);
		glBindTexture(GL_TEXTURE_2D, KEY_bedroom);
		glActiveTexture(GL_TEXTURE17);
		glBindTexture(GL_TEXTURE_2D, KEY_bathroom);
		glActiveTexture(GL_TEXTURE18);
		glBindTexture(GL_TEXTURE_2D, CHECKING);
		glActiveTexture(GL_TEXTURE19);
		glBindTexture(GL_TEXTURE_2D, PARTICLE1);
		glActiveTexture(GL_TEXTURE20);
		glBindTexture(GL_TEXTURE_2D, PARTICLE2);
		
		i = 1;
		for (auto& w : walls) {
			//if (i > 1 && !b_2d) continue;
			if (!w.active) { i++; continue; }
			w.setSize();
			GLint uloc;
			uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, w.model_matrix);
			glUniform1i(glGetUniformLocation(program, "TEX"), i);
			glUniform1i(glGetUniformLocation(program, "use_texture"), true);
			glUniform1i(glGetUniformLocation(program, "mode"), 1);

			if (i == 18 && !b_2d) continue;
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr); 
			
			if (i > 17) continue;
			i++;
		}
		if (b_game) {
			for (auto& p :particles) {
				p.update(t);
				
				GLint uloc;
				uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, p.model_matrix);
				uloc = glGetUniformLocation(program, "color");			if (uloc > -1) glUniform4fv(uloc, 1, p.color);
				glUniform1i(glGetUniformLocation(program, "TEX"), 19);
				glUniform1i(glGetUniformLocation(program, "use_texture"), true);
				glUniform1i(glGetUniformLocation(program, "mode"), 2);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
			}
		}
		if (b_kill&& (t - t_kill < dead_interval)) {
			for (auto& p : particles) {
				p.update(t);

				GLint uloc;
				uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, p.model_matrix);
				uloc = glGetUniformLocation(program, "color");			if (uloc > -1) glUniform4fv(uloc, 1, p.color);
				glUniform1i(glGetUniformLocation(program, "TEX"), 20);
				glUniform1i(glGetUniformLocation(program, "use_texture"), true);
				glUniform1i(glGetUniformLocation(program, "mode"), 2);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
			}
		}
		if (scene > 5 && scene < 11 && !b_help) {
			float dpi_scale = cg_get_dpi_scale();
			render_text("Energy: ", 20, 30, 0.5f, vec4(0.7f, 0.4f, 0.1f, 0.8f), dpi_scale);
			render_text(EnergyBar(), 120, 30, 0.5f, vec4(0.7f, 0.4f, 0.1f, 0.8f), dpi_scale);
			render_text("Left time: ", 400, 30, 0.5f, vec4(0.7f, 0.4f, 0.1f, 0.8f), dpi_scale);
			render_text(LeftTime(), 550, 30, 0.5f, vec4(0.7f, 0.4f, 0.1f, 0.8f), dpi_scale);
		
		}
	}
	skip:
	// swap front and back buffers, and display to screen
	glfwSwapBuffers(window);
}


#pragma endregion

#pragma region Initialize

void reshape(GLFWwindow* window, int width, int height)
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width, height);
	glViewport(0, 0, width, height);
}

void print_help()
{
	printf("[help]\n");
	printf("- press ESC or 'q' to terminate the program\n");
	printf("- press F1 or 'h' to see help\n");
	printf("- press Home to reset camera\n");
	printf("- press 'd' to toggle between OBJ format and 3DS format\n");
	printf("\n");
}

bool init_background()
{
	program_background = cg_create_program(vert_background_path, frag_background_path);
	if (!program_background) return false;

	vertex vertices[] =
	{	//-z
		{ vec3(-1.0f,1.0f,-1.0f) },
		{ vec3(-1.0f,-1.0f,-1.0f)},
		{ vec3(1.0f,-1.0f,-1.0f) },
		{ vec3(1.0f,-1.0f,-1.0f) },
		{ vec3(1.0f,1.0f,-1.0f)  },
		{ vec3(-1.0f,1.0f,-1.0f) },
		//+y
		{ vec3(-1.0f,1.0f,-1.0f) },
		{ vec3(1.0f,1.0f,-1.0f)  },
		{ vec3(1.0f,1.0f,1.0f)   },
		{ vec3(1.0f,1.0f,1.0f)   },
		{ vec3(-1.0f,1.0f,1.0f)  },
		{ vec3(-1.0f,1.0f,-1.0f) },
		//-y
		{ vec3(-1.0f,-1.0f,-1.0f)},
		{ vec3(-1.0f,-1.0f,1.0f) },
		{ vec3(1.0f,-1.0f,-1.0f) },
		{ vec3(1.0f,-1.0f,-1.0f) },
		{ vec3(-1.0f,-1.0f,1.0f) },
		{ vec3(1.0f,-1.0f,1.0f)  },
		//+z
		{ vec3(-1.0f,-1.0f,1.0f) },
		{ vec3(-1.0f,1.0f,1.0f)  },
		{ vec3(1.0f,1.0f,1.0f)   },
		{ vec3(1.0f,1.0f,1.0f)   },
		{ vec3(1.0f,-1.0f,1.0f)  },
		{ vec3(-1.0f,-1.0f,1.0f) },
		//-x
		{ vec3(-1.0f,-1.0f,1.0f) },
		{ vec3(-1.0f,-1.0f,-1.0f)},
		{ vec3(-1.0f,1.0f,-1.0f) },
		{ vec3(-1.0f,1.0f,-1.0f) },
		{ vec3(-1.0f,1.0f,1.0f)  },
		{ vec3(-1.0f,-1.0f,1.0f) },
		//+x
		{ vec3(1.0f,-1.0f,-1.0f) },
		{ vec3(1.0f,-1.0f,1.0f)  },
		{ vec3(1.0f,1.0f,1.0f)   },
		{ vec3(1.0f,1.0f,1.0f)   },
		{ vec3(1.0f,1.0f,-1.0f)  },
		{ vec3(1.0f,-1.0f,-1.0f) },
	};

	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	VAO_BACKGROUND = cg_create_vertex_array(vertex_buffer); if (!VAO_BACKGROUND) { printf("%s(): VAO==nullptr\n", __func__); return false; }
	skyboxTexture = loadCubemap(skyboxes);
	return true;
}
bool init_image()
{
	// create corners and vertices
	vertex corners[4];
	corners[0].pos = vec3(-1.0f, -1.0f, 0.5f);	corners[0].tex = vec2(0.0f, 0.0f);
	corners[1].pos = vec3(+1.0f, -1.0f, 0.5f);	corners[1].tex = vec2(1.0f, 0.0f);
	corners[2].pos = vec3(+1.0f, +1.0f, 0.5f);	corners[2].tex = vec2(1.0f, 1.0f);
	corners[3].pos = vec3(-1.0f, +1.0f, 0.5f);	corners[3].tex = vec2(0.0f, 1.0f);
	vertex vertices[6] = { corners[0], corners[1], corners[2], corners[0], corners[2], corners[3] };

	// generation of vertex buffer is the same, but use vertices instead of corners
	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	if (VAO_IMAGE) glDeleteVertexArrays(1, &VAO_IMAGE);
	VAO_IMAGE = cg_create_vertex_array(vertex_buffer);
	if (!VAO_IMAGE) { printf("%s(): failed to create vertex aray\n", __func__); return false; }

	program_img = cg_create_program(vert_image, frag_image);
	if (!program_img) return false;
	return true;
}
bool init_sound() {

	engine = irrklang::createIrrKlangDevice();
	if (!engine) return false;
	background_src = engine->addSoundSourceFromFile(absolute_path(background_sound));
	gameover_src = engine->addSoundSourceFromFile(absolute_path(gameover_sound));
	gameend_src = engine->addSoundSourceFromFile(absolute_path(gameend_sound));
	background_src->setDefaultVolume(0.3f);
	gameover_src->setDefaultVolume(0.3f);
	gameend_src->setDefaultVolume(0.7f);
	engine->play2D(background_src, true);
	return true;
}
bool user_init()
{
	// log hotkeys
	print_help();

	// init GL states
	glClearColor(39 / 255.0f, 40 / 255.0f, 34 / 255.0f, 1.0f);	// set clear color
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);								// turn on backface culling
	glEnable(GL_DEPTH_TEST);								// turn on depth tests
	glEnable(GL_TEXTURE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0);

	unit_wall_vertices = std::move(create_wall());
	update_vertex_buffer(unit_wall_vertices);
	WALL_warehouse = cg_create_texture(wall_warehouse, true); if (!WALL_warehouse) return false;
	WALL_living = cg_create_texture(wall_living, true); if (!WALL_living) return false;
	WALL_kitchen = cg_create_texture(wall_kitchen, true); if (!WALL_kitchen) return false;
	WALL_bedroom = cg_create_texture(wall_bedroom, true); if (!WALL_bedroom) return false;
	WALL_bathroom = cg_create_texture(wall_bathroom, true); if (!WALL_bathroom) return false;
	
	START = cg_create_texture(img_start, true); if (!START) return false;
	END = cg_create_texture(img_end, true); if (!END) return false;
	HELP = cg_create_texture(img_help, true); if (!START) return false;
	
	wall_tex[0] = WALL_warehouse; wall_tex[1] = WALL_living; wall_tex[2] = WALL_kitchen; wall_tex[3] = WALL_bedroom; wall_tex[4] = WALL_bathroom;

	DOOR_warehouse = cg_create_texture(object_door_warehouse, true); if (!DOOR_warehouse) return false;
	DOOR_living = cg_create_texture(object_door_living, true); if (!DOOR_living) return false;
	DOOR_kitchen = cg_create_texture(object_door_kitchen, true); if (!DOOR_kitchen) return false;
	DOOR_bedroom = cg_create_texture(object_door_bedroom, true); if (!DOOR_bedroom) return false;
	DOOR_bathroom = cg_create_texture(object_door_bathroom, true); if (!DOOR_bathroom) return false;

	BEACON_warehouse = cg_create_texture(beacon_warehouse, true); if (!BEACON_warehouse) return false;
	BEACON_living = cg_create_texture(beacon_living, true); if (!BEACON_living) return false;
	BEACON_kitchen = cg_create_texture(beacon_kitchen, true); if (!BEACON_kitchen) return false;
	BEACON_bedroom = cg_create_texture(beacon_bedroom, true); if (!BEACON_bedroom) return false;
	BEACON_bathroom = cg_create_texture(beacon_bathroom, true); if (!BEACON_bathroom) return false;

	KEY_warehouse = cg_create_texture(img_key_warehouse, true); if (!KEY_warehouse) return false;
	KEY_living = cg_create_texture(img_key_living, true); if (!KEY_living) return false;
	KEY_kitchen = cg_create_texture(img_key_kitchen, true); if (!KEY_kitchen) return false;
	KEY_bedroom = cg_create_texture(img_key_bedroom, true); if (!KEY_bedroom) return false;
	KEY_bathroom = cg_create_texture(img_key_bathroom, true); if (!KEY_bathroom) return false;

	CHECKING = cg_create_texture(img_checking, true); if (!CHECKING) return false;

	CHARGE = cg_create_texture(img_charge, true); if (!CHARGE) return false;

	PARTICLE1 = cg_create_texture(particle_explode, true); if (!PARTICLE1) return false;
	PARTICLE2 = cg_create_texture(particle_splash, true); if (!PARTICLE2) return false;
	// load the mesh
	pMesh.emplace_back(load_model(mesh_warehouse));
	pMesh.emplace_back(load_model(mesh_hero));
	pMesh.emplace_back(load_model(wood_box));
	pMesh.emplace_back(load_model(mesh_living));
	pMesh.emplace_back(load_model(mesh_kitchen));
	pMesh.emplace_back(load_model(mesh_bedroom));
	pMesh.emplace_back(load_model(mesh_bathroom));
	pMesh.emplace_back(load_model(mesh_flower));
	pMesh.emplace_back(load_model(mesh_warehouse_key));
	pMesh.emplace_back(load_model(mesh_living_key));
	pMesh.emplace_back(load_model(mesh_kitchen_key));
	pMesh.emplace_back(load_model(mesh_bedroom_key));
	pMesh.emplace_back(load_model(mesh_bathroom_key));
	pMesh.emplace_back(load_model(wood_doublebox));


	hero = &models[1];
	particles.resize(particle_t::MAX_PARTICLES);
	if (pMesh.empty()) { printf("Unable to load mesh\n"); return false; }
	if (!init_text()) return false;
	if (!init_background()) return false;
	if (!init_image()) return false;
	if (!init_sound()) return false;
	return true;
}

#pragma endregion

#pragma region User_Input

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE)	glfwSetWindowShouldClose(window, GL_TRUE);
		else if (key == GLFW_KEY_Q) {
			scene = 0;
			restart();
			in_game = false;
		}
		else if (key == GLFW_KEY_H || key == GLFW_KEY_F1) {
			if (scene > 5) {
				if (help_count < 3) {
					pause = true;
					in_game = false;
					b_help = true;
					help_count++;
				}
				else {
					pause = false;
					in_game = true;
					b_help = false;
					help_count = 0;
				}
			}
		}
		else if (key == GLFW_KEY_1) {
			if (scene == 5) {
				difficulty = 1;
				load_level(difficulty);
			}
		}
		else if (key == GLFW_KEY_2) {
			if (scene == 5) {
				difficulty = 2;
				load_level(difficulty);
			}
		}
		else if (key == GLFW_KEY_3) {
			if (scene == 5) {
				difficulty = 3;
				load_level(difficulty);
			}
		}
		else if (key == GLFW_KEY_B) {
			if (scene > 5) {
				load_level(difficulty);
			} 
			
		}
		else if (key == GLFW_KEY_HOME)					cam = camera();
		else if (key == GLFW_KEY_S && scene == 0)					scene = 1;
		else if (key == GLFW_KEY_N && scene != 0 && scene < 5) scene++;
			
		else if (key == GLFW_KEY_F && !b_game && in_game)
		{
			b_2d = !b_2d;
		}
		else if (key == GLFW_KEY_R) {
			if (scene > 5) {
				reset();
				restart();
			}
		}
		else if (key == GLFW_KEY_P) {
			pause = !pause;
			in_game = !in_game;
		}
		else if (key == GLFW_KEY_A && !b_game && in_game)
		{
			if (hero->action != PULL) hero->action = PUSH;
		}
		else if (key == GLFW_KEY_S && !b_game && in_game)
		{
			if (hero->action != PUSH) hero->action = PULL;
		}
		else if (key == GLFW_KEY_RIGHT && !b_game && in_game) {
			int move_type;
			if (!b_2d) {
				move_type = hero->right_move(cur_map, models, walls, keys);
			}
			else {
				move_type = hero->right_move_2d(cur_map, models, walls, keys);
			}
			if (move_type == 6) {
				key_capture();
			}
		}
		else if (key == GLFW_KEY_LEFT && !b_game && in_game) {
			int move_type;
			if (!b_2d) {
				move_type = hero->left_move(cur_map, models, walls, keys);

				if (move_type > 0 && move_type < 6) {
					capture(scene);

					switch (move_type) {
					case 1: //pink
						if (scene == 6) scene = 7;
						else scene = 6;
						break;
					case 2: //black
						printf("game clear!!!!!\n");
						b_clear = true;
						break;
					case 3: // yellow
						break;
					case 4: // red
						if (scene == 9) scene = 7;
						else scene = 9;
						break;
					case 5: // blue
						break;
					}

					load_state(scene);
					load_game_scene(scene);
				}
			} 
			else {
				move_type = hero->left_move_2d(cur_map, models, walls, keys);
			}

			if (move_type == 6) {
				key_capture();
			}
		}
		else if (key == GLFW_KEY_UP && !b_game && in_game) {
			if (!b_2d) {
				int move_type = hero->up_move(cur_map, models, walls, keys);

				if (move_type > 0 && move_type < 6) {
					capture(scene);

					switch (move_type) {
					case 1: //pink
						if (scene == 6) scene = 7;
						else scene = 6;
						break;
					case 2: //black
						printf("game clear!!!!!\n");
						b_clear = true;
						break;
					case 3: // yellow
						break;
					case 4: // red
						if (scene == 9) scene = 7;
						else scene = 9;
						break;
					case 5: // blue
						break;
					}

					load_state(scene);
					load_game_scene(scene);
				}

				if (move_type == 6) {
					key_capture();
				}
			} 
		}
		else if (key == GLFW_KEY_DOWN && !b_game && in_game) {
			if (!b_2d) {
				if (hero->down_move(cur_map, models, walls, keys) == 6) {
					key_capture();
				}
			} 
		}
		
	}

	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_A)
		{
			if (hero->action != PULL) hero->action = 0;
		}
		else if (key == GLFW_KEY_S)
		{
			if (hero->action != PUSH) hero->action = 0;
		}
	}
}

void mouse(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		dvec2 pos; glfwGetCursorPos(window, &pos.x, &pos.y);
		vec2 npos = cursor_to_ndc(pos, window_size);
		if (action == GLFW_PRESS)			tb.begin(cam.view_matrix, npos);
		else if (action == GLFW_RELEASE)	tb.end();
	}
}

void motion(GLFWwindow* window, double x, double y)
{
	if (!tb.is_tracking()) return;
	vec2 npos = cursor_to_ndc(dvec2(x, y), window_size);
	cam.view_matrix = tb.update(npos);
}

#pragma endregion


void user_finalize()
{
	delete_texture_cache();
	pMesh.clear();
	engine->drop();
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

