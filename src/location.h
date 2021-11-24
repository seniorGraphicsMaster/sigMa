#pragma once
#ifndef __LOCATION_H__
#define __LOCATION_H__

vec3 grid2pos(int scene, vec2 pos) {
	vec3 center;
	
	switch (scene) {
	case 6:
		center = vec3(-30 + 15 * pos.x, -67.5f + 15 * pos.y, 1.0f);
		break;
	case 7:
		center = vec3(-105 + 15 * pos.x, -105 + 15 * pos.y, 1.0f);
		break;
	case 8:
		center = vec3(-30 + 15 * pos.x, -67.5f + 15 * pos.y, 1.0f);
		break;
	case 9:
		center = vec3(-67.5f + 15 * pos.x, -67.5f + 15 * pos.y, 1.0f);
		break;
	case 10:
		center = vec3(-67.5f + 15 * pos.x, -30 + 15 * pos.y, 1.0f);
		break;
	}

	return center;
}

void obj_3d_pos(model_t& model, int scene, vec2 pos) {
	model.cur_pos = pos;
	model.center = grid2pos(scene, pos);
}

vec3 wall2pos(int scene, int direction, int pos, float height) {
	vec3 center;
	if (direction == 0) {
		switch (scene) {
		case 6:
			center = vec3(-39.48f, -67.5f + 15 * pos, 1.0f + height/2);
			break;
		case 7:
			center = vec3(-114.48f, -105.0f + 15 * pos, 1.0f + height / 2);
			break;
		case 8:
			center = vec3(-39.48f, -67.5f + 15 * pos, 1.0f + height / 2);
			break;
		case 9:
			center = vec3(-76.98f, -67.5f + 15 * pos, 1.0f + height / 2);
			break;
		case 10:
			center = vec3(-76.98f, -30.0f + 15 * pos, 1.0f + height / 2);
			break;
		}
	}
	else {
		switch (scene) {
		case 6:
			center = vec3(-30.0f + 15 * pos, 76.98f, 1.0f + height / 2);
			break;
		case 7:
			center = vec3(-105.0f + 15 * pos, 114.48f, 1.0f + height / 2);
			break;
		case 8:
			center = vec3(-30.0f + 15 * pos, 76.98f, 1.0f + height / 2);
			break;
		case 9:
			center = vec3(-67.5f + 15 * pos, 76.98f, 1.0f + height / 2);
			break;
		case 10:
			center = vec3(-67.5f + 15 * pos, 39.48f, 1.0f + height / 2);
			break;

		}
	}
	
	return center;
}

void obj_2d_pos(wall_t& wall, int scene, int direction, int pos, vec2 size) {
	if (direction == 0) wall.theta = 0;
	else wall.theta = - PI / 2;

	wall.size = vec2(15 * size.x, 15 * size.y);
	wall.center = wall2pos(scene, direction, pos, wall.size.y);
	wall.direction = direction;
	wall.wallpos = pos;
}

void obj_floor_pos(wall_t& wall, int scene, vec2 pos) {
	wall.theta = -PI / 2;
	wall.z_theta = -PI / 2;
	wall.pos = pos;

	wall.size = vec2(15.0f, 15.0f);

	switch (scene) {
	case 6:
		wall.center = vec3(-30.0f + 15 * pos.x, -67.5f + 15 * pos.y, 1.01f);
		break;
	case 7:
		wall.center = vec3(-105.0f + 15 * pos.x, -105.0f + 15 * pos.y, 1.01f);
		break;
	case 8:
		wall.center = vec3(-30.0f + 15 * pos.x, -67.5f + 15 * pos.y, 1.01f);
		break;
	case 9:
		wall.center = vec3(-67.5f + 15 * pos.x, -67.5f + 15 * pos.y, 1.01f);
		break;
	case 10:
		wall.center = vec3(-67.5f + 15 * pos.x, -30.0f + 15 * pos.y, 1.01f);
		break;
	}

}



#endif