#pragma once
#ifndef __WALL_H__
#define __WALL_H__


struct wall_t
{
	int		id = 0;
	vec3	center = vec3(0);		// 2D position for translation
	vec2	size = vec2(0);
	bool	active = false;
	vec2	pos = vec2(-1, -1);
	int		direction = -1;
	int		wallpos = -1;
	int		wallpos_z = -1;

	float	theta = 0.0f;
	float	z_theta = 0.0f;

	mat4	model_matrix;

	void	setSize();
};
inline std::vector<wall_t> set_wall() {
	std::vector<wall_t> arr;
	wall_t m;
	m = {0, vec3(-39.49f,0.0f,26.0f),vec2(154.0f,50.0f), true };//warehouse
	arr.emplace_back(m);
	m = {0, vec3(-39.48f,52.5f,21.0f),vec2(15.0f,20.0f), true };//door 1
	arr.emplace_back(m);
	m = {0, vec3(-39.48f,52.5f,21.0f),vec2(15.0f,20.0f), false };//door 2
	arr.emplace_back(m);
	m = {0, vec3(-39.48f,52.5f,21.0f),vec2(15.0f,20.0f), false };//door 3
	arr.emplace_back(m);
	m = {0, vec3(-39.48f,52.5f,21.0f),vec2(15.0f,20.0f), false };//door 4
	arr.emplace_back(m);
	m = {0, vec3(-39.48f,52.5f,21.0f),vec2(15.0f,20.0f), false };//door 5
	arr.emplace_back(m);
	m = {1, vec3(-39.48f,52.5f,21.0f),vec2(15.0f,20.0f), false };//beacon 1
	arr.emplace_back(m);
	m = {2, vec3(-39.48f,52.5f,21.0f),vec2(15.0f,20.0f), false };//beacon 2
	arr.emplace_back(m);
	m = {3, vec3(-39.48f,52.5f,21.0f),vec2(15.0f,20.0f), false };//beacon 3
	arr.emplace_back(m);
	m = {4, vec3(-39.48f,52.5f,21.0f),vec2(15.0f,20.0f), false };//beacon 4
	arr.emplace_back(m);
	m = {5, vec3(-39.48f,52.5f,21.0f),vec2(15.0f,20.0f), false };//beacon 5
	arr.emplace_back(m);
	m = { 0, vec3(-39.48f,52.5f,21.0f),vec2(15.0f,20.0f), false };//charge
	arr.emplace_back(m);
	m = { 5, vec3(-39.48f,52.5f,21.0f),vec2(15.0f,20.0f), false };//key 1
	arr.emplace_back(m);
	//m = { 6, vec3(-39.48f,52.5f,21.0f),vec2(15.0f,20.0f), false };//key 2
	arr.emplace_back(m);
	//m = { 7, vec3(-39.48f,52.5f,21.0f),vec2(15.0f,20.0f), false };//key 3
	arr.emplace_back(m);
	//m = { 8, vec3(-39.48f,52.5f,21.0f),vec2(15.0f,20.0f), false };//key 4
	arr.emplace_back(m);
	//m = { 9, vec3(-39.48f,52.5f,21.0f),vec2(15.0f,20.0f), false };//key 5
	arr.emplace_back(m);
	
	return arr;
}
inline void wall_t::setSize()
{

	float c = cos(theta), s = sin(theta);
	float cz = cos(z_theta), sz = sin(z_theta);

	mat4 scale_matrix =
	{
		1, 0, 0, 0,
		0, size.x, 0, 0,
		0, 0, size.y, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		c, -s, 0, 0,
		s, c, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 z_rotation_matrix =
	{
		1, 0, 0, 0,
		0, cz, -sz, 0,
		0, sz, cz, 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, center.x,
		0, 1, 0, center.y,
		0, 0, 1, center.z,
		0, 0, 0, 1
	};

	model_matrix = translate_matrix * z_rotation_matrix * rotation_matrix * scale_matrix;
}


#endif
