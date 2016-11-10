
#pragma once

#include <SDL_ttf.h>
#include <SDL_opengl.h>
#include <vector>
#include <string>
#include "glfuns.h"

using namespace std;

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
	"	color = texture(tex, vec2(coord.x, 1.0f - coord.y));\n"
	"}\n"
};

struct point {
	float x, y, tx, ty;
};

struct UItext {
	UItext(TTF_Font* f, string s) {
		font = f;
		SDL_Surface* temp = TTF_RenderText_Shaded(f, s.c_str(), { 0, 0, 0 }, { 255, 255, 255 });
		surf = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_RGB888, 0);
		SDL_FreeSurface(temp);
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glGenTextures(1, &texture);
		glGenBuffers(1, &VBO);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindVertexArray(0);
	}
	~UItext() {
		SDL_FreeSurface(surf);
		glDeleteBuffers(1, &VBO);
		glDeleteTextures(1, &texture);
		glDeleteVertexArrays(1, &VAO);
	}
	void send() {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
		glBindVertexArray(0);
	}
	void render(GLuint program) {
		send();
		glBindVertexArray(VAO);
		glUseProgram(program);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		glBindTexture(GL_TEXTURE_2D, texture);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
	void reload(string s) {
		SDL_FreeSurface(surf);
		SDL_Surface* temp = TTF_RenderText_Shaded(font, s.c_str(), { 0, 0, 0 }, { 255, 255, 255 });
		surf = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_RGB888, 0);
		SDL_FreeSurface(temp);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	void update(SDL_Event* e) {

	}
	SDL_Surface* surf;
	TTF_Font* font;
	GLuint texture, VBO, VAO;
	point points[6];
};

struct UI {
	vector<UItext*> elements;
	GLuint program;
	int editing;
	~UI() {
		for (auto e : elements)
			delete e;
	}
	void init() {
		GLuint vert, frag;
		vert = glCreateShader(GL_VERTEX_SHADER);
		frag = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(vert, 1, &vtextured2D, NULL);
		glShaderSource(frag, 1, &ftextured2D, NULL);
		glCompileShader(vert);
		glCompileShader(frag);
		program = glCreateProgram();
		glAttachShader(program, vert);
		glAttachShader(program, frag);
		glLinkProgram(program); 
		glDeleteShader(vert);
		glDeleteShader(frag);
	}
	void render() {
		for (auto e : elements) {
			e->render(program);
		}
	}
	void update(SDL_Event* ev) {
		for (auto e : elements) {
			e->update(ev);
		}
	}
};