
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>

using namespace glm;

struct cam {
	vec3 pos, front, up, right, globalUp;
	float pitch, yaw, speed, fov;
};

mat4 getView(cam c) {
	mat4 ret = lookAt(c.pos, c.pos + c.front, c.up);
	ret = rotate(ret, radians(90.0f), vec3(1, 0, 0));
	return ret;
}

void updoot(cam& c) {
	vec3 front;
	front.x = cos(radians(c.pitch)) * cos(radians(c.yaw));
	front.y = sin(radians(c.pitch));
	front.z = sin(radians(c.yaw)) * cos(radians(c.pitch));
	c.front = normalize(front);
	c.right = normalize(cross(front, c.globalUp));
	c.up = normalize(cross(c.right, c.front));
}

cam defaultCam() {
	cam c;
	c.pitch = 0.0f;
	c.yaw = -20.0f;
	c.speed = 0.5f;
	c.fov = 90.0f;
	c.pos = vec3(-5, -5, 0);
	c.globalUp = vec3(0, 1, 0);
	updoot(c);
	return c;
}