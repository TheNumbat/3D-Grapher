
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
#include "tex_in.data"
#include "tex_out.data"
#include "tex_gear.data"

using namespace glm;
using namespace std;

typedef int op;

TTF_Font* font;

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
	virtual int render(int w, int h, int ui_w, int x, int y, shader& program) = 0;
	virtual bool process(SDL_Event ev, int w, state* s) = 0;
	point pts[6];
	int current_y, current_yh;
	bool active, should_remove;
};

struct UI {
	UI() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		active = false;
		in.tex.load(SDL_LoadBMP_RW(SDL_RWFromConstMem(in_bmp, in_bmp_len), 1));
		out.tex.load(SDL_LoadBMP_RW(SDL_RWFromConstMem(out_bmp, out_bmp_len), 1));
		gear.tex.load(SDL_LoadBMP_RW(SDL_RWFromConstMem(gear_bmp, gear_bmp_len), 1));
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
		int xoff, ui_w = (int)round(w * UI_SCREEN_RATIO);
		if (active) {
			xoff = 0;
			in.set(ui_w + 5, 0, 32, 32);
			in.render(w, h, ui_s);
			gear.set(ui_w + 5, 35, 32, 32);
			gear.render(w, h, ui_s);
		}
		else {
			xoff = -ui_w + 5;
			out.set(11, 0, 32, 32);
			out.render(w, h, ui_s);
		}
		drawRect(rect_s, xoff, 0, ui_w, h, 1.0f, 1.0f, 1.0f, 1.0f, fw, fh); // white background
		drawRect(rect_s, xoff + ui_w, 0, 3, h, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // right black strip
		int cur_y = 0;
		unsigned int windex = 0;
		while (cur_y < h && windex < widgets.size()) {
			drawRect(rect_s, xoff, cur_y, ui_w, 3, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // top/bottom black strip
			cur_y += 3;
			cur_y = widgets[windex++]->render(w, h, ui_w, 5 + xoff, cur_y, ui_s); // widget
			cur_y += 3;
		}
		drawRect(rect_s, xoff, cur_y, ui_w, 3, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // bottom black strip
	}
	vector<widget*> widgets;
	GLuint VAO, VBO;
	textured_rect in, out, gear;
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
	UI* ui;
	inputstate instate;

	bool running;
};