
#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <assert.h>
#include <SDL.h>
#include <GL\glew.h>

struct state {
	SDL_GLContext context;
	GLuint shader, VAO, VBO;
	SDL_Window* window;
	int w, h;
	bool running;
};

void setup(state* s, int w, int h);

const GLchar* vertex = {
	"#version 330 core\n"

	"layout(location = 0) in vec3 position;\n"
	"layout (location = 1) in vec3 color;\n"

	"out vec3 vcolor;\n"

	"uniform mat4 model, view, proj;\n"

	"void main() {\n"
	"	gl_Position = proj * view * model * vec4(position, 1.0f);\n"
	"	vcolor = color;\n"
	"}\n"
};

const GLchar* fragment = {
	"#version 330 core\n"

	"in vec3 vcolor;\n"
	"out vec4 color;\n"

	"void main() {\n"
	"	color = vec4(vcolor, 1.0f);\n"
	"}\n"
};

const GLfloat cube[] = {
	-0.5f, -0.5f, -0.5f,  	0.5f, 0.5f, 0.5f,
	 0.5f, -0.5f, -0.5f,  	0.5f, 0.5f, 0.5f,
	 0.5f,  0.5f, -0.5f,  	0.5f, 0.5f, 0.5f,
	 0.5f,  0.5f, -0.5f,  	0.5f, 0.5f, 0.5f,
	-0.5f,  0.5f, -0.5f,  	0.5f, 0.5f, 0.5f,
	-0.5f, -0.5f, -0.5f,  	0.5f, 0.5f, 0.5f,

	-0.5f, -0.5f,  0.5f,  	0.5f, 0.5f, 0.5f,
	 0.5f, -0.5f,  0.5f,  	0.5f, 0.5f, 0.5f,
	 0.5f,  0.5f,  0.5f,  	0.5f, 0.5f, 0.5f,
	 0.5f,  0.5f,  0.5f,  	0.5f, 0.5f, 0.5f,
	-0.5f,  0.5f,  0.5f,  	0.5f, 0.5f, 0.5f,
	-0.5f, -0.5f,  0.5f,  	0.5f, 0.5f, 0.5f,

	-0.5f,  0.5f,  0.5f,  	0.5f, 0.5f, 0.5f,
	-0.5f,  0.5f, -0.5f,  	0.5f, 0.5f, 0.5f,
	-0.5f, -0.5f, -0.5f,  	0.5f, 0.5f, 0.5f,
	-0.5f, -0.5f, -0.5f,  	0.5f, 0.5f, 0.5f,
	-0.5f, -0.5f,  0.5f,  	0.5f, 0.5f, 0.5f,
	-0.5f,  0.5f,  0.5f,  	0.5f, 0.5f, 0.5f,

	 0.5f,  0.5f,  0.5f,  	0.5f, 0.5f, 0.5f,
	 0.5f,  0.5f, -0.5f,  	0.5f, 0.5f, 0.5f,
	 0.5f, -0.5f, -0.5f,  	0.5f, 0.5f, 0.5f,
	 0.5f, -0.5f, -0.5f,  	0.5f, 0.5f, 0.5f,
	 0.5f, -0.5f,  0.5f,  	0.5f, 0.5f, 0.5f,
	 0.5f,  0.5f,  0.5f,  	0.5f, 0.5f, 0.5f,

	-0.5f, -0.5f, -0.5f,  	0.5f, 0.5f, 0.5f,
	 0.5f, -0.5f, -0.5f,  	0.5f, 0.5f, 0.5f,
	 0.5f, -0.5f,  0.5f,  	0.5f, 0.5f, 0.5f,
	 0.5f, -0.5f,  0.5f,  	0.5f, 0.5f, 0.5f,
	-0.5f, -0.5f,  0.5f,  	0.5f, 0.5f, 0.5f,
	-0.5f, -0.5f, -0.5f,  	0.5f, 0.5f, 0.5f,

	-0.5f,  0.5f, -0.5f,  	0.5f, 0.5f, 0.5f,
	 0.5f,  0.5f, -0.5f,  	0.5f, 0.5f, 0.5f,
	 0.5f,  0.5f,  0.5f,  	0.5f, 0.5f, 0.5f,
	 0.5f,  0.5f,  0.5f,  	0.5f, 0.5f, 0.5f,
	-0.5f,  0.5f,  0.5f,  	0.5f, 0.5f, 0.5f,
	-0.5f,  0.5f, -0.5f,  	0.5f, 0.5f, 0.5f
};