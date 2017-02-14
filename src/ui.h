
#pragma once

#include "gl.h"
#include "graph.h"
#include <string>
#include <vector>
#include <functional>

using namespace std;

enum ui_state {
	ui_funcs,
	ui_settings,
	ui_funcs_adding
};

struct state;

struct graph_enter_callback {
	graph_enter_callback(int i, graph_type g);
	void operator()(state* s, string e) const;
	int g_id;
	graph_type gt;
};

struct graph_remove_callback {
	graph_remove_callback(int i);
	bool operator()(state* s) const;
	int g_id;
};

struct widget {
	virtual ~widget() {};
	virtual int render(state* s, int ui_w, int x, int y) = 0;
	virtual bool update(state* s, SDL_Event* ev) = 0;
	point pts[6];
	int current_y, current_yh, current_x, current_xw;
	bool active, should_remove;
};

struct edit_text;

struct UI {
	UI(state* s);
	~UI();
	void remove_dead_widgets();
	void drawRect(shader& shader, int x, int y, int w, int h, float r, float g, float b, float a, float screenw, float screenh);
	void render(state* s);
	void render_sidebar(state* s);
	int render_widgets(state* s, vector<widget*>& v, int ui_w, int x, int y, bool fullborders);
	bool parseDoms(state* s);
	vector<widget*> funcs, funcs_add, settings;
	vector<widget*> dom_rect, dom_cyl, dom_spr; // ONLY EDIT_TEXT
	GLuint VAO, VBO;
	textured_rect in_r, out_r, gear_r, f_r, q_r, error_r;
	string error;
	vector<textured_rect*> helpText;
	vector<string> help;
	ui_state uistate;
	graph_type domain;
	bool active;
	bool helpShown;
	bool errorShown;
	int adding_x, adding_y;
};

struct edit_text : public widget {
	edit_text(state* s, string e, string h, function<void(state*, string)> c, function<bool(state*)> rm, bool a = true);
	int render(state* s, int ui_w, int x, int y);
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
	int render(state* s, int ui_w, int x, int y);
	bool update(state* s, SDL_Event* ev);
	function<void(state*)> toggleCallback;
	bool on;
	string text;
	textured_rect r;
};

struct static_text : public widget {
	static_text(string t, function<void(state*)> c);
	~static_text() {}
	int render(state* s, int ui_w, int x, int y);
	bool update(state* s, SDL_Event* ev);
	function<void(state*)> clickCallback;
	string text;
	textured_rect r;
};

struct multi_text : public widget {
	multi_text(vector<string> strs, int p, function<void(state*, string)> c);
	~multi_text() {}
	int render(state* s, int ui_w, int x, int y);
	bool update(state* s, SDL_Event* ev);
	function<void(state*, string)> toggleCallback;
	int pos;
	vector<string> text;
	textured_rect r;
};

struct slider : public widget {
	slider(string t, float f, function<void(state*, float)> c);
	~slider() {}
	int render(state* s, int ui_w, int x, int y);
	bool update(state* s, SDL_Event* ev);
	function<void(state*, float)> moveCallback;
	string text;
	float pos;
	int slider_w;
	textured_rect r;
};