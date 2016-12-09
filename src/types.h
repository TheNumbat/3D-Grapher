
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

#include "gl.h"

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
		current_y = current_yh = 0;
		active = should_remove = false;
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
	void gl_render(shader& program) {
		glDisable(GL_BLEND);
		glBindVertexArray(VAO);
		program.use();

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		glBindTexture(GL_TEXTURE_2D, texture);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glBindVertexArray(0);
	}
	virtual int render(int y_pos, int w, int total_w, int h, int xoffset, shader& program) = 0;
	virtual bool process(SDL_Event ev, int w, state* s) = 0;
	point pts[6];
	GLuint VAO, VBO, texture;
	int current_y, current_yh;
	bool active, should_remove;
};

struct UI {
	UI() {}
	void start() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		active = false;
	}
	~UI() {
		for (widget* w : widgets)
			delete w;
		widgets.clear();
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}
	void remove_dead_widgets() {
		for (unsigned int i = 0; i < widgets.size(); i++) {
			if (widgets[i]->should_remove) {
				delete widgets[i];
				widgets.erase(widgets.begin() + i);
			}
		}
	}
	void drawRect(shader& shader, int x, int y, int w, int h, float r, float g, float b, float a, float screenw, float screenh) {
		glEnable(GL_BLEND);
		glBindVertexArray(VAO);
		shader.use();
		GLfloat pts[12] = { -1.0f + 2.0f * (x / screenw)      ,  1.0f - 2.0f * (y / screenh),
			                -1.0f + 2.0f * ((x + w) / screenw),  1.0f - 2.0f * (y / screenh),
			                -1.0f + 2.0f * ((x + w) / screenw),  1.0f - 2.0f * ((y + h) / screenh),

			                -1.0f + 2.0f * ((x + w) / screenw),  1.0f - 2.0f * ((y + h) / screenh),
			                -1.0f + 2.0f * (x / screenw)      ,  1.0f - 2.0f * ((y + h) / screenh),
			                -1.0f + 2.0f * (x / screenw)      ,  1.0f - 2.0f * (y / screenh) };
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(pts), pts, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glUniform4f(shader.getUniform("vcolor"), r, g, b, a);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(0);
		glBindVertexArray(0);
	}
	void render(int w, int h, shader& ui_s, shader& rect_s) {
		float fw = (float)w, fh = (float)h;
		if (active) {
			int ui_w = (int)round(w * UI_SCREEN_RATIO);
			drawRect(rect_s, 0, 0, ui_w, h, 1.0f, 1.0f, 1.0f, 1.0f, fw, fh);
			drawRect(rect_s, ui_w, 0, 3, h, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh);
			int cur_y = 0;
			unsigned int windex = 0;
			while (cur_y < h && windex < widgets.size()) {
				drawRect(rect_s, 0, cur_y, ui_w, 3, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh);
				cur_y += 3;
				cur_y = widgets[windex++]->render(cur_y, ui_w, w, h, 5, ui_s);
				cur_y += 3;
			}
			drawRect(rect_s, 0, cur_y, ui_w, 3, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh);
			drawRect(rect_s, ui_w, 0, 25, 25, 0.0f, 0.0f, 0.0f, 0.5f, fw, fh);
		}
		else {
			drawRect(rect_s, 0, 0, 25, 25, 0.0f, 0.0f, 0.0f, 0.5f, fw, fh);
		}
	}
	vector<widget*> widgets;
	GLuint VAO, VBO;
	bool active;
};

struct state {
	SDL_Window* window;
	int w, h;
	SDL_GLContext context;
	GLuint axisVAO, graphVAO, axisVBO, graphVBO, EBO;
	shader graph_s, axis_s, UI_s, rect_s;

	graph g;
	cam c;
	UI ui;
	inputstate instate;

	bool running;
};