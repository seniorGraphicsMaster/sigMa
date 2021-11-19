#pragma once
#ifndef __WALL_H__
#define __WALL_H__


struct wall_t
{
	vec3	center = vec3(0);		// 2D position for translation
	float	scale;

	mat4	model_matrix;

	void	setSize();
};
inline std::vector<wall_t> set_wall() {
	std::vector<wall_t> arr;
	wall_t m;
	m = { vec3(-39.49f,0.0f,26.0f),154.0f };//warehouse
	arr.emplace_back(m);
	return arr;
}
inline void wall_t::setSize()
{

	mat4 scale_matrix =
	{
		1, 0, 0, 0,
		0, scale, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
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
