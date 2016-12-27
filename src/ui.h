
#pragma once

#include "gl.h"
#include <string>
#include <vector>

using namespace std;

enum ui_state {
	ui_funcs,
	ui_settings
};

struct state;

struct widget {
	virtual ~widget() {};
	virtual int render(state* s, int w, int h, int ui_w, int x, int y, shader& program) = 0;
	virtual bool update(state* s, SDL_Event* ev) = 0;
	point pts[6];
	int current_y, current_yh;
	bool active, should_remove;
};

struct UI {
	UI();
	~UI();
	void remove_dead_widgets();
	void drawRect(shader& shader, int x, int y, int w, int h, float r, float g, float b, float a, float screenw, float screenh);
	void render(state* s, int w, int h, shader& ui_s, shader& rect_s);
	vector<widget*> widgets;
	GLuint VAO, VBO;
	textured_rect in, out, gear, f;
	ui_state uistate;
	bool active;
};

struct fxy_equation : public widget {
	fxy_equation(int graph_id, bool a = false);
	~fxy_equation() {}
	int render(state* s, int w, int h, int ui_w, int x, int y, shader& program);
	bool update(state* s, SDL_Event* ev);
	void break_str(state* s, int w);
	int g_id;
	string exp;
	vector<string> lines;
	textured_rect r;
};
