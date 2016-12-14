
#pragma once

#include <SDL_opengl.h>

const GLchar* graph_vertex = {
	"#version 330 core\n"

	"layout (location = 0) in vec3 position;\n"

	"uniform mat4 modelviewproj;\n"

	"void main() {\n"
	"	gl_Position = modelviewproj * vec4(position, 1.0f);\n"
	"}\n"
};

const GLchar* graph_fragment = {
	"#version 330 core\n"

	"uniform vec4 vcolor;\n"
	"out vec4 color;\n"

	"void main() {\n"
	"	color = vcolor;\n"
	"}\n"
};

const GLchar* ui_vertex = {
	"#version 330 core\n"

	"layout (location = 0) in vec2 position;\n"
	"layout (location = 1) in vec2 tcoord;\n"

	"out vec2 coord;\n"

	"void main() {\n"
	"	gl_Position = vec4(position, 0.0f, 1.0f);\n"
	"	coord = tcoord;\n"
	"}\n"
};

const GLchar* ui_fragment = {
	"#version 330 core\n"

	"in vec2 coord;\n"
	"out vec4 color;\n"
	"uniform sampler2D tex;\n"

	"void main() {\n"
	"	color = texture(tex, vec2(coord.x, 1.0f - coord.y));\n"
	"}\n"
};

const GLchar* rect_vertex = {
	"#version 330 core\n"

	"layout (location = 0) in vec2 position;\n"

	"uniform vec4 vcolor;\n"
	"out vec4 fcolor;\n"

	"void main() {\n"
	"	gl_Position = vec4(position, 0.0f, 1.0f);\n"
	"	fcolor = vcolor;\n"
	"}\n"
};

const GLchar* rect_fragment = {
	"#version 330 core\n"

	"in vec4 fcolor;\n"
	"out vec4 color;\n"

	"void main() {\n"
	"	color = fcolor;\n"
	"}\n"
};

const GLchar* axis_vertex = {
	"#version 330 core\n"

	"layout (location = 0) in vec3 position;\n"
	"layout (location = 1) in vec3 color;\n"

	"out vec3 vcolor;\n"

	"uniform mat4 modelviewproj;\n"

	"void main() {\n"
	"	gl_Position = modelviewproj * vec4(position, 1.0f);\n"
	"	vcolor = color;\n"
	"}\n"
};

const GLchar* axis_fragment = {
	"#version 330 core\n"

	"in vec3 vcolor;\n"
	"out vec4 color;\n"

	"void main() {\n"
	"	color = vec4(vcolor, 1.0f);\n"
	"}\n"
};