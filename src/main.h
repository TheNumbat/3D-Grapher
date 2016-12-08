
#pragma once

#include "font.data"
#include "types.h"

#include "glfuns.h"
#include "exp.h"
#include "cam.h"
#include "graph.h"
#include "ui.h"

const GLchar* colorvertex = {
	"#version 330 core\n"

	"layout (location = 0) in vec3 position;\n"
	"layout (location = 1) in vec3 color;\n"

	"out vec3 vcolor;\n"

	"uniform mat4 model, view, proj;\n"

	"void main() {\n"
	"	gl_Position = proj * view * model * vec4(position, 1.0f);\n"
	"	vcolor = color;\n"
	"}\n"
};

const GLchar* vertex = {
	"#version 330 core\n"

	"layout (location = 0) in vec3 position;\n"

	"uniform mat4 model, view, proj;\n"

	"void main() {\n"
	"	gl_Position = proj * view * model * vec4(position, 1.0f);\n"
	"}\n"
};

const GLchar* colorfragment = {
	"#version 330 core\n"

	"in vec3 vcolor;\n"
	"out vec4 color;\n"

	"void main() {\n"
	"	color = vec4(vcolor, 1.0f);\n"
	"}\n"
};

const GLchar* fragment = {
	"#version 330 core\n"

	"uniform vec4 vcolor;\n"
	"out vec4 color;\n"

	"void main() {\n"
	"	color = vcolor;\n"
	"}\n"
};

// index into axes
const int x_min = 0;
const int x_max = 6;
const int y_min = 13;
const int y_max = 19;
const int z_min = 26;
const int z_max = 32;

GLfloat axes[] = {
	 0.0f, 0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
	 0.0f, 0.0f, 0.0f,  	1.0f, 0.0f, 0.0f,

	 0.0f, 0.0f, 0.0f,  	0.0f, 1.0f, 0.0f,
	 0.0f, 0.0f, 0.0f,  	0.0f, 1.0f, 0.0f,

	 0.0f, 0.0f, 0.0f,  	0.0f, 0.0f, 1.0f,
	 0.0f, 0.0f, 0.0f,  	0.0f, 0.0f, 1.0f
};
