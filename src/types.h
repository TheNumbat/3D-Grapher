
#pragma once

enum inputstate {
	in_idle,
	in_cam,
	in_text
};

#include <vector>
#include <functional>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_opengl.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

typedef int op;

TTF_Font* font;

struct point {
	float x, y, tx, ty;
};

struct cam {
	vec3 pos, front, up, right, globalUp;
	float pitch, yaw, speed, fov;
	Uint32 lastUpdate;
};

struct graph {
	vector<op> eq;
	string eq_str;
	vector<GLfloat> verticies;
	vector<GLuint> indicies;
	float xmin, xmax, ymin, ymax;
	unsigned int xrez, yrez;
};

struct state;
struct gendata {
	gendata() {
		zmin = FLT_MAX;
		zmax = -FLT_MAX;
	};
	state* s;
	vector<float> ret;
	float zmin, zmax, xmin, dx, dy;
	unsigned int txrez;
};

struct widget {
	widget() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenTextures(1, &texture);
	}
	~widget() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteTextures(1, &texture);
	}
	void send() {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(pts), pts, GL_STATIC_DRAW);
		glBindVertexArray(0);
	}
	void gl_render(GLuint program) {
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
	virtual int render(int y_pos, int w, int h, int xoffset, GLuint program) = 0;
	virtual void process(SDL_Event ev) = 0;
	function<void(state*)> callback;
	point pts[6];
	GLuint VAO, VBO, texture;
};

struct UI {
	UI() {

	}
	~UI() {
		for (widget* w : widgets)
			delete w;
		widgets.clear();
	}
	void render(int w, int h, GLuint program) {
		int cur_y = 0, windex = 0;
		while (cur_y < h && windex < widgets.size()) {
			glScissor(0, h - cur_y - 3, w, 3);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glScissor(0, 0, w, h);
			cur_y += 3;
			cur_y = widgets[windex++]->render(cur_y, w, h, 5, program);
			cur_y += 3;
		}
		glScissor(0, h - cur_y - 3, w, 3);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glScissor(0, 0, w, h);
		cur_y += 3;
	}
	vector<widget*> widgets;
};

struct state {
	SDL_Window* window;
	int w, h;
	SDL_GLContext context;
	GLuint axisShader, graphShader, uiShader, axisVAO, graphVAO, axisVBO, graphVBO, EBO;

	graph g;
	cam c;
	UI ui;
	inputstate instate;

	bool running;
};