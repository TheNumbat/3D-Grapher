
#pragma once

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

using namespace glm;

enum cam_type {
	cam_3d,
	cam_3d_static
};

struct _cam_3d {
	vec3 pos, front, up, right, globalUp;
	float pitch, yaw, speed;
	Uint32 lastUpdate;

	mat4 getView() {
		mat4 ret = lookAt(pos, pos + front, up);
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

	void reset() {
		pitch = -45.0f;
		yaw = 225.0f;
		speed = 5.0f;
		pos = vec3(5, 5, 5);
		globalUp = vec3(0, 1, 0);
		lastUpdate = SDL_GetTicks();
		updateFront();
	}

	void move(int dx, int dy) {
		const float sens = 0.1f;
		yaw += dx * sens;
		pitch -= dy * sens;
		if (yaw > 360.0f) yaw = 0.0f;
		else if (yaw < 0.0f) yaw = 360.0f;
		if (pitch > 89.0f) pitch = 89.0f;
		else if (pitch < -89.0f) pitch = -89.0f;
		updateFront();
	}
};

struct _cam_3d_static {
	vec3 pos, lookingAt, up;
	float pitch, yaw, radius;

	mat4 getView() {
		mat4 ret = lookAt(pos, lookingAt, up);
		return ret;
	}

	void reset() {
		pitch = 45.0f;
		radius = 20.0f;
		lookingAt = vec3(0, 0, 0);
		up = vec3(0, 1, 0);
		updatePos();
	}

	void updatePos() {
		pos.x = cos(radians(pitch)) * cos(radians(yaw));
		pos.y = sin(radians(pitch));
		pos.z = sin(radians(yaw)) * cos(radians(pitch));
		pos = radius * normalize(pos) + lookingAt;
	}

	void move(int dx, int dy) {
		const float sens = 0.1f;
		yaw += dx * sens;
		pitch -= dy * sens;
		if (yaw > 360.0f) yaw = 0.0f;
		else if (yaw < 0.0f) yaw = 360.0f;
		if (pitch > 89.0f) pitch = 89.0f;
		else if (pitch < -89.0f) pitch = -89.0f;
		updatePos();
	}
};

