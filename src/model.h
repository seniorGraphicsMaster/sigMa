#pragma once
#ifndef __MODEL_H__
#define __MODEL_H__

#include "map.h"

#define CANMOVE 0
#define OBJMOVEIDX 2
#define PUSH 100
#define PULL 200

struct model_t
{
	//init var
	vec3	center=vec3(0);		// 2D position for translation
	float	scale;

	//move var
	vec2	cur_pos=vec2(0);
	float	theta = 0.0f;			// rotation angle
	float	time = 0.0f;			// check time
	mat4	model_matrix;		// modeling transformation

	int		action = 0;

	// public functions
	void	update(float t);
	void	left_move(map_t& cur_map, std::vector<model_t>& models);
	void	right_move(map_t& cur_map, std::vector<model_t>& models);
	void	up_move(map_t& cur_map, std::vector<model_t>& models);
	void	down_move(map_t& cur_map, std::vector<model_t>& models);
};

inline std::vector<model_t> set_pos() {
	std::vector<model_t> arr;
	model_t m;
	m = {vec3(0.0f,0.0f,0.0f),1.0f};//warehouse
	arr.emplace_back(m);
	m = {vec3(0.0f,-7.5f,1.0f),1.0f, vec2(2,4)};//hero
	arr.emplace_back(m);
	m = { vec3(-15.0f,-22.5f,1.0f),1.0f, vec2(1,3) };//wood_box
	arr.emplace_back(m);
	m = { vec3(15.0f, 22.5f,1.0f),1.0f, vec2(3,6) };//wood_box
	arr.emplace_back(m);
	return arr;
}

inline void model_t::left_move(map_t& cur_map, std::vector<model_t>& models){
	vec2 next_pos = vec2(cur_pos.x - 1, cur_pos.y);
	
	//rotation
	if (action == PULL) theta = PI / 2;
	else theta = -PI / 2;
	
	//wall check
	if (next_pos.x < 0) return;
	
	//can't move
	int next_val = cur_map.map[int(next_pos.x)][int(next_pos.y)];
	if (action != PUSH && next_val != CANMOVE) return;

	//push
	if (action == PUSH && next_val > OBJMOVEIDX - 1) {
		model_t* obj = &models[next_val];
		vec2 obj_next_pos = vec2(obj->cur_pos.x - 1, obj->cur_pos.y);
		if (obj_next_pos.x < 0) return;
		
		int obj_next_val = cur_map.map[int(obj_next_pos.x)][int(obj_next_pos.y)];
		if (obj_next_val != CANMOVE) return;

		obj->cur_pos = obj_next_pos;
		obj->center.x -= 15.0f;
		cur_map.map[int(next_pos.x)][int(next_pos.y)] = 0;
		cur_map.map[int(obj_next_pos.x)][int(obj_next_pos.y)] = next_val;
	}

	//pull
	vec2 pre_pos = vec2(cur_pos.x + 1, cur_pos.y);
	if (pre_pos.x < cur_map.grid.x) {
		int pre_val = cur_map.map[int(pre_pos.x)][int(pre_pos.y)];
		if (action == PULL && pre_val > OBJMOVEIDX - 1) {
			model_t* obj = &models[pre_val];

			obj->cur_pos = cur_pos;
			obj->center.x -= 15.0f;
			cur_map.map[int(pre_pos.x)][int(pre_pos.y)] = 0;
			cur_map.map[int(cur_pos.x)][int(cur_pos.y)] = pre_val;
		}
	}
	
	cur_pos = next_pos;
	center.x -= 15.0f;
}

inline void model_t::right_move(map_t& cur_map, std::vector<model_t>& models) {
	vec2 next_pos = vec2(cur_pos.x + 1, cur_pos.y);

	//rotation
	if (action == PULL) theta = - PI / 2;
	else theta = PI / 2;

	//wall check
	if (next_pos.x > cur_map.grid.x - 1) return;

	//can't move
	int next_val = cur_map.map[int(next_pos.x)][int(next_pos.y)];
	if (action != PUSH && next_val != CANMOVE) return;

	//push
	if (action == PUSH && next_val > OBJMOVEIDX - 1) {
		model_t* obj = &models[next_val];
		vec2 obj_next_pos = vec2(obj->cur_pos.x + 1, obj->cur_pos.y);
		if (obj_next_pos.x > cur_map.grid.x - 1) return;
		
		int obj_next_val = cur_map.map[int(obj_next_pos.x)][int(obj_next_pos.y)];
		 if(obj_next_val != CANMOVE) return;

		obj->cur_pos = obj_next_pos;
		obj->center.x += 15.0f;
		cur_map.map[int(next_pos.x)][int(next_pos.y)] = 0;
		cur_map.map[int(obj_next_pos.x)][int(obj_next_pos.y)] = next_val;
	}

	//pull
	vec2 pre_pos = vec2(cur_pos.x - 1, cur_pos.y);
	if (pre_pos.x > -1) {
		int pre_val = cur_map.map[int(pre_pos.x)][int(pre_pos.y)];
		if (action == PULL && pre_val > OBJMOVEIDX - 1) {
			model_t* obj = &models[pre_val];

			obj->cur_pos = cur_pos;
			obj->center.x += 15.0f;
			cur_map.map[int(pre_pos.x)][int(pre_pos.y)] = 0;
			cur_map.map[int(cur_pos.x)][int(cur_pos.y)] = pre_val;
		}
	}

	cur_pos = next_pos;
	center.x += 15.0f;

}

inline void model_t::up_move(map_t& cur_map, std::vector<model_t>& models) {
	
	vec2 next_pos = vec2(cur_pos.x, cur_pos.y + 1);

	//rotation
	if (action == PULL) theta = 0;
	else theta = PI;

	//wall check
	if (next_pos.y > cur_map.grid.y - 1) return;

	//can't move
	int next_val = cur_map.map[int(next_pos.x)][int(next_pos.y)];
	if (action != PUSH && next_val != CANMOVE) return;

	//push
	if (action == PUSH && next_val > OBJMOVEIDX - 1) {
		model_t* obj = &models[next_val];
		vec2 obj_next_pos = vec2(obj->cur_pos.x, obj->cur_pos.y + 1);
		if (obj_next_pos.y > cur_map.grid.y - 1) return;

		int obj_next_val = cur_map.map[int(obj_next_pos.x)][int(obj_next_pos.y)];
		if (obj_next_val != CANMOVE) return;

		obj->cur_pos = obj_next_pos;
		obj->center.y += 15.0f;
		cur_map.map[int(next_pos.x)][int(next_pos.y)] = 0;
		cur_map.map[int(obj_next_pos.x)][int(obj_next_pos.y)] = next_val;
	}

	//pull
	vec2 pre_pos = vec2(cur_pos.x, cur_pos.y - 1);
	if (pre_pos.y > -1) {
		int pre_val = cur_map.map[int(pre_pos.x)][int(pre_pos.y)];
		if (action == PULL && pre_val > OBJMOVEIDX - 1) {
			model_t* obj = &models[pre_val];

			obj->cur_pos = cur_pos;
			obj->center.y += 15.0f;
			cur_map.map[int(pre_pos.x)][int(pre_pos.y)] = 0;
			cur_map.map[int(cur_pos.x)][int(cur_pos.y)] = pre_val;
		}
	}

	cur_pos = next_pos;
	center.y += 15.0f;

}
inline void model_t::down_move(map_t& cur_map, std::vector<model_t>& models) {
	
	vec2 next_pos = vec2(cur_pos.x, cur_pos.y - 1);

	//rotation
	if (action == PULL) theta = PI;
	else theta = 0;

	//wall check
	if (next_pos.y < 0) return;

	//can't move
	int next_val = cur_map.map[int(next_pos.x)][int(next_pos.y)];
	if (action != PUSH && next_val != CANMOVE) return;

	//push
	if (action == PUSH && next_val > OBJMOVEIDX - 1) {
		model_t* obj = &models[next_val];
		vec2 obj_next_pos = vec2(obj->cur_pos.x, obj->cur_pos.y - 1);
		if (obj_next_pos.y < 0) return;
		
		int obj_next_val = cur_map.map[int(obj_next_pos.x)][int(obj_next_pos.y)];
		if (obj_next_val != CANMOVE) return;

		obj->cur_pos = obj_next_pos;
		obj->center.y -= 15.0f;
		cur_map.map[int(next_pos.x)][int(next_pos.y)] = 0;
		cur_map.map[int(obj_next_pos.x)][int(obj_next_pos.y)] = next_val;
	}

	//pull
	vec2 pre_pos = vec2(cur_pos.x, cur_pos.y + 1);
	if (pre_pos.y < cur_map.grid.y) {
		int pre_val = cur_map.map[int(pre_pos.x)][int(pre_pos.y)];
		if (action == PULL && pre_val > OBJMOVEIDX - 1) {
			model_t* obj = &models[pre_val];

			obj->cur_pos = cur_pos;
			obj->center.y -= 15.0f;
			cur_map.map[int(pre_pos.x)][int(pre_pos.y)] = 0;
			cur_map.map[int(cur_pos.x)][int(cur_pos.y)] = pre_val;
		}
	}

	cur_pos = next_pos;
	center.y -= 15.0f;
}

inline void model_t::update(float t)
{
	/*if (!r) {
		time = t - theta;
	}
	else {
		theta = t - time;
	}*/

	float c = cos(theta), s = sin(theta);

	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		scale, 0, 0, 0,
		0, scale, 0, 0,
		0, 0, scale, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		c, -s, 0, 0,
		s, c, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, center.x,
		0, 1, 0, center.y,
		0, 0, 1, center.z,
		0, 0, 0, 1
	};

	model_matrix = translate_matrix * rotation_matrix * scale_matrix;
}

#endif
