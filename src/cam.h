
#pragma once

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

using namespace glm;

struct cam {
	vec3 pos, front, up, right, globalUp;
	float pitch, yaw, speed, fov;
	Uint32 lastUpdate;

	mat4 getView() {
		mat4 ret = lookAt(pos, pos + front, up);
		ret = rotate(ret, radians(-90.0f), vec3(1, 0, 0));
		return ret;
	}

	void updateFront() {
		front.x = cos(radians(pitch)) * cos(radians(yaw));
		front.y = sin(radians(pitch));
		front.z = sin(radians(yaw)) * cos(radians(pitch));
		front = normalize(front);
		right = normalize(cross(front, globalUp));
		up = normalize(cross(right, front));
	}

	void default() {
		pitch = -45.0f;
		yaw = 225.0f;
		speed = 5.0f;
		fov = 90.0f;
		pos = vec3(5, 5, 5);
		globalUp = vec3(0, 1, 0);
		lastUpdate = SDL_GetTicks();
		updateFront();
	}
};