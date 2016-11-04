
#pragma once

#include <vector>

#include <assert.h>
#include <SDL.h>
#include <SDL_image.h>
#include <GL\glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cam.h"
#include "exp.h"

#include "texture.data"

using namespace glm;
using namespace std;

struct graph {
	vector<op> eq;
	float xmin, xmax, ymin, ymax;
	unsigned int xrez, yrez;
};

struct state {
	SDL_Window* window;
	int w, h;
	SDL_GLContext context;
	GLuint axisShader, graphShader, uiShader, VAO, axisVBO, graphVBO, uiVBO, EBO;
	GLuint texture;
	
	vector<GLfloat> verticies;
	vector<GLuint> indicies;

	graph g;
	cam c;

	bool running;
};

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

const GLchar* vtextured2D = {
	"#version 330 core\n"

	"layout (location = 0) in vec2 position;\n"
	"layout (location = 1) in vec2 tcoord;\n"

	"out vec2 coord;\n"

	"void main() {\n"
	"	gl_Position = vec4(position, 0.0f, 1.0f);\n"
	"	coord = tcoord;\n"
	"}\n"
};

const GLchar* ftextured2D{
	"#version 330 core\n"

	"in vec2 coord;\n"
	"out vec4 color;\n"
	"uniform sampler2D tex;\n"

	"void main() {\n"
	"	color = texture(tex, 1.0f - coord);\n"
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

const GLfloat uitest[] = {
	-0.5f,  0.5f,		0.0f, 1.0f,
	-0.5f, -0.5f,		0.0f, 0.0f,
	 0.5f,  0.5f,		1.0f, 1.0f,

	 0.5f,  0.5f,		1.0f, 1.0f,
	-0.5f, -0.5f,		0.0f, 0.0f,
	 0.5f, -0.5f,		1.0f, 0.0f
};