#pragma once
#ifndef __MODEL_H__
#define __MODEL_H__


struct model_t
{
	vec3	center=vec3(0);		// 2D position for translation
	float	scale;
	float	theta = 0.0f;			// rotation angle
	float	time = 0.0f;				// check time
	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update(float t);
};
inline std::vector<model_t> set_pos() {
	std::vector<model_t> arr;
	model_t m;
	m = {vec3(5.0f,5.0f,0.0f),1.0f};//warehouse
	arr.emplace_back(m);
	m = {vec3(0.0f,0.0f,1.0f),1.0f };//hero
	arr.emplace_back(m);
	return arr;
}

inline void model_t::update(float t)
{
	/*if (!r) {
		time = t - theta;
	}
	else {
		theta = t - time;
	}*/

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
