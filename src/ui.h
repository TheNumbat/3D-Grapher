
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

enum UItype {
	ui_button,
	ui_text,
	ui_dyntext
};

struct UIelement {
	struct point {
		float x, y, tx, ty;
	};
	virtual void render(GLuint VAO, GLuint VBO) = 0;
	virtual void update(SDL_Event* e) = 0;
	union {
		point points[6];
		float verts[24];
	};
	UItype type;
};

struct UIbutton : public UIelement {
	UIbutton() {
		type = ui_button;
	}
	virtual ~UIbutton() {}
	void render(GLuint VAO, GLuint VBO) {

	}
	SDL_Surface* texture, hover, clicked;
};

struct UIdyntext : public UIelement {
	UIdyntext(TTF_Font* f, string& s) : text(s) {
		type = ui_dyntext;
		glGenTextures(1, &texture);
		glGenBuffers(1, &VBO);
		font = f;
	}
	~UIdyntext() {
		glDeleteBuffers(1, &VBO);
		glDeleteTextures(1, &texture);
	}
	void render(GLuint VAO, GLuint program) {
		glBindVertexArray(VAO);
		glUseProgram(program);
		
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		SDL_Surface* temp = TTF_RenderText_Shaded(font, text.c_str(), { 0, 0, 0 }, { 255, 255, 255 });
		SDL_Surface* surf = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_RGB888, 0);

		glBindTexture(GL_TEXTURE_2D, texture);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		SDL_FreeSurface(temp);
		SDL_FreeSurface(surf);
	}
	void update(SDL_Event* e) {

	}
	string& text;
	GLuint texture, VBO;
	TTF_Font* font;
};

struct UItext : public UIelement {
	UItext(TTF_Font* f, string s) {
		type = ui_text;
		SDL_Surface* temp = TTF_RenderText_Shaded(f, s.c_str(), { 0, 0, 0 }, { 255, 255, 255 });
		surf = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_RGB888, 0);
		SDL_FreeSurface(temp);
		glGenTextures(1, &texture);
		glGenBuffers(1, &VBO);
	}
	~UItext() {
		SDL_FreeSurface(surf);
		glDeleteBuffers(1, &VBO);
		glDeleteTextures(1, &texture);
	}
	void render(GLuint VAO, GLuint program) {
		glBindVertexArray(VAO);
		glUseProgram(program);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	void update(SDL_Event* e) {

	}
	SDL_Surface* surf;
	GLuint texture, VBO;
};

struct UI {
	vector<UIelement*> elements;
	GLuint VAO, program;
	int editing;
	~UI() {
		glDeleteVertexArrays(1, &VAO);
		for (auto e : elements)
			delete e;
	}
	void init() {
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

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
			e->render(VAO, program);
		}
	}
	void update(SDL_Event* ev) {
		for (auto e : elements) {
			e->update(ev);
		}
	}
};