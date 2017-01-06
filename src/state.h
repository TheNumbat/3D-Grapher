
#pragma once

const float UI_SCREEN_RATIO = 0.2f;
const int screen_w = 1280, screen_h = 720;

#include <SDL2/SDL_ttf.h>
#include "graph.h"
#include "cam.h"
#include "ui.h"
#include "evts.h"
#include "settings.h"

struct state {
	state();
	~state();
	void run();

	SDL_Window* window;
	TTF_Font* font;
	SDL_GLContext context;

	int w, h, mx, my, next_graph_id, last_mx, last_my;
	bool running;

	GLuint axisVAO, axisVBO;
	shader graph_s, axis_s, UI_s, rect_s, graph_s_light;
	
	vector<graph*> graphs;
	_cam_3d c_3d;
	_cam_3d_static c_3d_static;
	UI* ui;
	evts ev;
	settings set;
};