
#pragma once

enum class cam_type {
	_3d = 0,
	_3d_static
};

struct _cam_3d {
	glm::vec3 pos, front, up, right, globalUp;
	float pitch, yaw, speed, fov;
	Uint32 lastUpdate;

	glm::mat4 getView() {
		glm::mat4 ret = lookAt(pos, pos + front, up);
		return ret;
	}

	void updateFront() {
		front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = normalize(front);
		right = normalize(cross(front, globalUp));
		up = normalize(cross(right, front));
	}

	void reset() {
		fov = 60.0f;
		pitch = -45.0f;
		yaw = 225.0f;
		speed = 5.0f;
		pos = glm::vec3(5, 5, 5);
		globalUp = glm::vec3(0, 1, 0);
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
	glm::vec3 pos, lookingAt, up;
	float pitch, yaw, radius, fov;

	glm::mat4 getView() {
		glm::mat4 ret = lookAt(pos, lookingAt, up);
		return ret;
	}

	void setAxis(glm::vec3 axis) {

		if(axis == glm::vec3(0,0,1)) {
			pitch = 90.0f;
			yaw = 0.0f;
		} else if(axis == glm::vec3(0,1,0)) {
			pitch = 0.0f;
			yaw = 0.0f;
		} else if(axis == glm::vec3(1,0,0)) {
			pitch = 0.0f;
			yaw = 90.0f;
		}
		updatePos();
	}

	void reset() {
		fov = 60.0f;
		pitch = 45.0f;
		yaw = 45.0f;
		radius = 20.0f;
		lookingAt = glm::vec3(0, 0, 0);
		up = glm::vec3(0, 1, 0);
		updatePos();
	}

	void updatePos() {
		pos.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		pos.y = sin(glm::radians(pitch));
		pos.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		pos = radius * glm::normalize(pos) + lookingAt;
	}

	void move(int dx, int dy) {
		const float sens = 0.1f;
		yaw += dx * sens;
		pitch -= dy * sens;
		if (yaw > 360.0f) yaw = 0.0f;
		else if (yaw < 0.0f) yaw = 360.0f;
		if (pitch > 89.9f) pitch = 89.9f;
		else if (pitch < -89.9f) pitch = -89.9f;
		updatePos();
	}
};

