
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

struct UIelement {
	UIelement() {
		glGenVertexArrays(1, &VAO);
		glGenTextures(1, &texture);
		glGenBuffers(1, &VBO);
	}
	virtual ~UIelement() {
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
	GLuint texture, VBO, VAO;
	point points[6];
};

struct UItexture : public UIelement {
	UItexture(SDL_Surface* tex) {
		glBindVertexArray(VAO);
		surf = SDL_ConvertSurfaceFormat(tex, SDL_PIXELFORMAT_RGB888, 0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindVertexArray(0);
	}
	~UItexture() {
		SDL_FreeSurface(surf);
	}
	SDL_Surface* surf;
};

struct UItext : public UIelement {
	UItext(TTF_Font* f, string s) {
		font = f;
		SDL_Surface* temp = TTF_RenderText_Shaded(f, s.c_str(), { 0, 0, 0 }, { 255, 255, 255 });
		surf = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_RGB888, 0);
		SDL_FreeSurface(temp);

		glBindVertexArray(VAO);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindVertexArray(0);
	}
	~UItext() {
		SDL_FreeSurface(surf);
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
	SDL_Surface* surf;
	TTF_Font* font;
};

struct graphelement {
	int pxoffset;
	int pxoffset_bot;
	vector<int> UIelements;
	string str;
	bool operator==(const graphelement& other) {
		return pxoffset == other.pxoffset && pxoffset_bot == other.pxoffset_bot && str == other.str;
	}
};

struct UI {
	vector<UIelement*> elements;
	vector<graphelement> gelements;
	graphelement* selected;
	GLuint program;
	int editing;
	~UI() {
		reset();
	}
	vector<string> reset() {
		for (UIelement* e : elements)
			delete e;
		vector<string> ret;
		for (graphelement& ge : gelements)
			ret.push_back(ge.str);
		elements.clear();
		gelements.clear();
		return ret;
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
	void render(int w, int h) {
		for (auto g : gelements) {
			for(int i : g.UIelements)
				elements[i]->render(program);
		}
		for (auto g : gelements) {
			glScissor(0, (int)(h - g.pxoffset_bot / 2 - 10), (int)(round(0.25f * w) - 3), 3);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glScissor(0, 0, w, h);
		}
	}
};