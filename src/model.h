#pragma once
#ifndef __MODEL_H__
#define __MODEL_H__

#include "map.h"

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

	// public functions
	void	update(float t);
	void	left_move(map_t map);
	void	right_move(map_t map);
	void	up_move(map_t map);
	void	down_move(map_t map);
};

inline std::vector<model_t> set_pos() {
	std::vector<model_t> arr;
	model_t m;
	m = {vec3(0.0f,0.0f,0.0f),1.0f};//warehouse
	arr.emplace_back(m);
	m = {vec3(0.0f,-7.5f,1.0f),1.0f, vec2(2,4)};//hero
	arr.emplace_back(m);
	return arr;
}

inline void model_t::left_move(map_t cur_map){
	vec2 next_pos = vec2(cur_pos.x - 1, cur_pos.y);
	int next_val = cur_map.map[int(next_pos.x)][int(next_pos.y)];

	theta = -PI / 2;
	if (next_pos.x < 0 || next_val == 2) return;
	
	cur_pos = next_pos;
	center.x -= 15.0f;
}
inline void model_t::right_move(map_t cur_map) {
	vec2 next_pos = vec2(cur_pos.x + 1, cur_pos.y);
	int next_val = cur_map.map[int(next_pos.x)][int(next_pos.y)];

	theta = PI / 2;
	if (next_pos.x > cur_map.grid.x - 1 || next_val == 2) return;
	
	cur_pos = next_pos;
	center.x += 15.0f;
}
inline void model_t::up_move(map_t cur_map) {
	vec2 next_pos = vec2(cur_pos.x, cur_pos.y + 1);
	int next_val = cur_map.map[int(next_pos.x)][int(next_pos.y)];

	theta = PI;
	if (next_pos.y > cur_map.grid.y - 1 || next_val == 2) return;
	
	cur_pos = next_pos;
	center.y += 15.0f;
}
inline void model_t::down_move(map_t cur_map) {
	vec2 next_pos = vec2(cur_pos.x, cur_pos.y - 1);
	int next_val = cur_map.map[int(next_pos.x)][int(next_pos.y)];

	theta = 0;
	if (next_pos.y < 0 || next_val == 2) return;
	
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
