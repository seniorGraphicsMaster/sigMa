#ifndef __PARTICLE_H__
#define __PARTICLE_H__
#pragma once

#include "cgmath.h"
#include "cgut.h"

inline float random_range( float min, float max ){ return mix( min, max, rand()/float(RAND_MAX) ); }

struct particle_t
{
	static constexpr int MAX_PARTICLES = 300;

	vec3 pos;
	vec4 color;
	vec3 velocity;
	vec3 accel;
	float scale;
	float life;
	float start=0;
	//optional
	float elapsed_time;
	float time_interval;

	mat4 model_matrix;

	particle_t() {  }
	particle_t(vec3 coord, float t,int type) { if(type) explode(coord,t); else splash(coord, t);}
	void explode(vec3 coord, float t);
	void splash(vec3 coord, float t);
	void update(float t);
};

inline void particle_t::explode(vec3 coord,float t)
{
	pos = vec3(random_range(coord.x - 4.0f, coord.x + 4.0f), random_range(coord.y - 4.0f, coord.y + 4.0f), random_range(coord.z - 4.0f, coord.z + 4.0f));
	color = vec4(random_range(0.5f, 1.0f), random_range(0, 0.3f), random_range(0, 0.2f), 1);
	scale = random_range(0.3f, 1.0f);
	life = random_range(0.01f, 1.5f);
	velocity = vec3(pos.x - coord.x, pos.y - coord.y, pos.z-coord.z) * 0.05f;
	accel = velocity * -0.1f;
	elapsed_time = 0.0f;
	time_interval = random_range(0.1f, 0.3f);
	start = t;
}
inline void particle_t::splash(vec3 coord, float t)
{
	pos = vec3(random_range(coord.x - 4.0f, coord.x + 4.0f), random_range(coord.y - 4.0f, coord.y + 4.0f), random_range(coord.z, coord.z + 5.0f));
	color = vec4(random_range(0.0f, 0.2f), random_range(0.5f, 1.0f), random_range(0, 0.4f), 1);
	scale = random_range(0.3f, 1.0f);
	life = random_range(0.01f, 1.5f);
	velocity = vec3(pos.x - coord.x, pos.y - coord.y, pos.z - coord.z) * 0.02f;
	velocity.z = velocity.z * 3.0f;
	accel = vec3(0.0f);
	accel.z = -0.06f;
	elapsed_time = 0.0f;
	time_interval = random_range(0.1f, 0.3f);
	start = t;
}

inline void particle_t::update(float t)
{
	float dwTime = t - start;
	elapsed_time = dwTime;
	if (elapsed_time > time_interval)
	{
		velocity = velocity + accel;
		scale = scale * 0.95f;
		elapsed_time = 0.0f;
		start = t;
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