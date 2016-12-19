
#pragma once

#include <vector>
#include <functional>

enum inputstate {
	in_idle,
	in_cam,
	in_ui
};

struct callback {
	callback(function<bool(state*, SDL_Event*)> f, inputstate s) {
		func = f;
		state = s;
	}
	function<bool(state*, SDL_Event*)> func;
	inputstate state;
};

struct evts {
	evts() {
		current = in_idle;
	}
	void run(state* s) {
		const static unsigned char* keys = SDL_GetKeyboardState(NULL);
		SDL_Event ev;
		while (SDL_PollEvent(&ev) != 0) {
			Uint8 flags = 0;
			switch (ev.type) {
			case SDL_QUIT:
				s->running = false;
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				for (callback& c : key)
					if (c.state == current)
						if(c.func(s, &ev))
							break;
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEMOTION:
			case SDL_MOUSEWHEEL:
				for (callback& c : mouse)
					if (c.state == current)
						if(c.func(s, &ev))
							break;
				break;
			case SDL_WINDOWEVENT:
				if (ev.window.event == SDL_WINDOWEVENT_RESIZED ||
					ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
					s->w = ev.window.data1;
					s->h = ev.window.data2;
					glViewport(0, 0, s->w, s->h);
					glScissor(0, 0, s->w, s->h);
				}
				break;
			}
		}
		float dT = (SDL_GetTicks() - s->c.lastUpdate) / 1000.0f;
		s->c.lastUpdate = SDL_GetTicks();
		if (current == in_cam) {
			if (keys[SDL_SCANCODE_W]) {
				s->c.pos += s->c.front * s->c.speed * dT;
			}
			if (keys[SDL_SCANCODE_S]) {
				s->c.pos -= s->c.front * s->c.speed * dT;
			}
			if (keys[SDL_SCANCODE_A]) {
				s->c.pos -= s->c.right * s->c.speed * dT;
			}
			if (keys[SDL_SCANCODE_D]) {
				s->c.pos += s->c.right * s->c.speed * dT;
			}
		}
	}
	inputstate current;
	vector<callback> mouse;
	vector<callback> key;
};