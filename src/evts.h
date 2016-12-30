
#pragma once

#include <vector>
#include <functional>
#include <SDL.h>

using namespace std;

struct state;

enum inputstate {
	in_idle,
	in_cam,
	in_funcs,
	in_widget,
	in_settings,
	in_any
};

struct callback {
	callback(function<bool(state*, SDL_Event*)> f, inputstate s, SDL_EventType t);
	function<bool(state*, SDL_Event*)> func;
	inputstate instate;
	SDL_EventType type;
};

struct evts {
	evts();
	void run(state* s);
	inputstate current;
	vector<callback> callbacks;
};

void add_default_callbacks(state* s);