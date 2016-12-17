
#pragma once

using namespace glm;

mat4 getView(cam c) {
	mat4 ret = lookAt(c.pos, c.pos + c.front, c.up);
	ret = rotate(ret, radians(-90.0f), vec3(1, 0, 0));
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
	c.pitch = -45.0f;
	c.yaw = 225.0f;
	c.speed = 5.0f;
	c.fov = 90.0f;
	c.pos = vec3(5, 5, 5);
	c.globalUp = vec3(0, 1, 0);
	c.lastUpdate = SDL_GetTicks();
	updoot(c);
	return c;
}