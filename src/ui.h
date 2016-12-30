
#pragma once

#include "gl.h"
#include <string>
#include <vector>
#include <functional>

using namespace std;

enum ui_state {
	ui_funcs,
	ui_settings
};

struct state;

struct fxy_enter_callback {
	fxy_enter_callback(int i);
	void operator()(state* s, string e) const;
	int g_id;
};

struct fxy_remove_callback {
	fxy_remove_callback(int i);
	bool operator()(state* s) const;
	int g_id;
};

struct widget {
	virtual ~widget() {};
	virtual int render(state* s, int w, int h, int ui_w, int x, int y) = 0;
	virtual bool update(state* s, SDL_Event* ev) = 0;
	point pts[6];
	int current_y, current_yh;
	bool active, should_remove;
};

struct UI {
	UI(state* s);
	~UI();
	void remove_dead_widgets();
	void drawRect(shader& shader, int x, int y, int w, int h, float r, float g, float b, float a, float screenw, float screenh);
	void render(state* s, int w, int h);
	void UI::render_sidebar(state* s);
	vector<widget*> funcs;
	vector<widget*> settings;
	GLuint VAO, VBO;
	textured_rect in, out, gear, f;
	ui_state uistate;
	bool active;
};

struct edit_text : public widget {
	edit_text(state* s, string e, string h, function<void(state*, string)> c, function<bool(state*)> rm, bool a = true);
	int render(state* s, int w, int h, int ui_w, int x, int y);
	bool update(state* s, SDL_Event* ev);
	void break_str(state* s);
	void update_cursor(state* s);
	void remove(state* s);
	int currentLine();
	int currentPos();
	function<void(state*, string)> enterCallback;
	function<bool(state*)> removeCallback;
	string head, exp;
	vector<string> lines;
	textured_rect r;
	int cursor_pos, cursor_x, cursor_y; // updated by update()
};

struct toggle_text : public widget {
	toggle_text(string t, bool o, function<void(state*)> c);
	~toggle_text() {}
	int render(state* s, int w, int h, int ui_w, int x, int y);
	bool update(state* s, SDL_Event* ev);
	function<void(state*)> toggleCallback;
	bool on;
	string text;
	textured_rect r;
};

struct multi_text : public widget {
	multi_text(vector<string> strs, int p, function<void(state*, string)> c);
	~multi_text() {}
	int render(state* s, int w, int h, int ui_w, int x, int y);
	bool update(state* s, SDL_Event* ev);
	function<void(state*, string)> toggleCallback;
	int pos;
	vector<string> text;
	textured_rect r;
};

struct slider : public widget {
	slider(string t, float f, function<void(state*, float)> c);
	~slider() {}
	int render(state* s, int w, int h, int ui_w, int x, int y);
	bool update(state* s, SDL_Event* ev);
	function<void(state*, float)> moveCallback;
	string text;
	float pos;
	int slider_w;
	textured_rect r;
};