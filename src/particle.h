#ifndef __PARTICLE_H__
#define __PARTICLE_H__
#pragma once

#include "cgmath.h"
#include "cgut.h"

inline float random_range( float min, float max ){ return mix( min, max, rand()/float(RAND_MAX) ); }

struct particle_t
{
	static constexpr int MAX_PARTICLES = 200;

	vec3 pos;
	vec4 color;
	vec3 velocity;
	float scale;
	float life;
	float start=0;
	//optional
	float elapsed_time;
	float time_interval;

	mat4 model_matrix;

	particle_t() { reset(); }
	particle_t(vec3 coord, float t) { explode(coord,t); }
	void reset();
	void reset(float t);
	void explode(vec3 coord, float t);
	void update();
};
inline void particle_t::reset()
{
	pos = vec3(random_range(-1.0f, 1.0f), random_range(-1.0f, 1.0f), random_range(-1.0f, 1.0f));
	color = vec4(random_range(0, 1.0f), random_range(0, 1.0f), random_range(0, 1.0f), 1);
	scale = random_range(0.005f, 0.08f);
	life = random_range(0.01f, 1.0f);
	velocity = vec3(random_range(-1.0f, 1.0f), random_range(-1.0f, 1.0f), random_range(-1.0f, 1.0f)) * 0.003f;
	elapsed_time = 0.0f;
	time_interval = random_range(200.0f, 600.0f);
}
inline void particle_t::reset(float t)
{
	pos = vec3(random_range(-1.0f, 1.0f), random_range(-1.0f, 1.0f), random_range(-1.0f, 1.0f));
	color = vec4(random_range(0.5f, 1.0f), random_range(0, 1.0f), random_range(0, 1.0f), 1);
	scale = random_range(0.005f, 0.08f);
	life = random_range(0.01f, 1.0f);
	velocity = vec3(random_range(-1.0f, 1.0f), random_range(-1.0f, 1.0f), random_range(-1.0f, 1.0f)) * 0.003f;
	elapsed_time = 0.0f;
	time_interval = random_range(200.0f, 600.0f);
	start = t;
}

inline void particle_t::explode(vec3 coord,float t)
{
	pos = vec3(random_range(coord.x - 5.0f, coord.x + 5.0f), random_range(coord.y - 5.0f, coord.y + 5.0f), random_range(coord.z - 5.0f, coord.z + 5.0f));
	color = vec4(random_range(0.5f, 1.0f), random_range(0, 0.2f), random_range(0, 0.2f), 1);
	scale = random_range(0.3f, 3.0f);
	life = random_range(0.01f, 1.5f);
	velocity = vec3(pos.x - coord.x, pos.y - coord.y, pos.z-coord.z) * 0.03f;
	elapsed_time = 0.0f;
	time_interval = random_range(200.0f, 600.0f);
	start = t;
}

inline void particle_t::update()
{
	const float dwTime = (float)glfwGetTime() - start;
	elapsed_time += dwTime;

	if (elapsed_time > time_interval)
	{
		//const float theta = random_range(0, 1.0f) * PI * 2.0f;
		constexpr float velocity_factor = 0.3f;
		//velocity = vec2(cos(theta), sin(theta)) * velocity_factor;
		velocity = velocity * velocity_factor;
		scale = scale * 0.95f;
		elapsed_time = 0.0f;
	}

	pos += velocity;

	constexpr float life_factor = 0.001f;
	life -= life_factor * dwTime;

	// disappear
	if (life < 0.0f)
	{
		constexpr float alpha_factor = 0.001f;
		color.a -= alpha_factor * dwTime;
	}
	float c = cos(0.0f), s = sin(0.0f);
	mat4 scale_matrix =
	{
		1, 0, 0, 0,
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
		1, 0, 0, pos.x,
		0, 1, 0, pos.y,
		0, 0, 1, pos.z,
		0, 0, 0, 1
	};

	model_matrix = translate_matrix *  rotation_matrix * scale_matrix;
	// deadr
	//if (color.a < 0.0f) reset();
}

#endif