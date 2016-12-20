
#pragma once

#include <vector>
#include <functional>

#include "cam.h"

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
			case SDL_TEXTINPUT:
				for (callback& c : text)
					if (c.state == current)
						if (c.func(s, &ev))
							break;
				break;
			case SDL_KEYDOWN:
				for (callback& c : keydown)
					if (c.state == current)
						if (c.func(s, &ev))
							break;
				break;
			case SDL_KEYUP:
				for (callback& c : keyup)
					if (c.state == current)
						if(c.func(s, &ev))
							break;
				break;
			case SDL_MOUSEBUTTONDOWN:
				for (callback& c : mousedown)
					if (c.state == current)
						if (c.func(s, &ev))
							break;
				break;
			case SDL_MOUSEBUTTONUP:
				for (callback& c : mouseup)
					if (c.state == current)
						if (c.func(s, &ev))
							break;
				break;
			case SDL_MOUSEMOTION:
				for (callback& c : mousemotion)
					if (c.state == current)
						if (c.func(s, &ev))
							break;
				break;
			case SDL_MOUSEWHEEL:
				for (callback& c : mousewheel)
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
		float dT = (SDL_GetTicks() - s->c->lastUpdate) / 1000.0f;
		s->c->lastUpdate = SDL_GetTicks();
		if (current == in_cam) {
			if (keys[SDL_SCANCODE_W]) {
				s->c->pos += s->c->front * s->c->speed * dT;
			}
			if (keys[SDL_SCANCODE_S]) {
				s->c->pos -= s->c->front * s->c->speed * dT;
			}
			if (keys[SDL_SCANCODE_A]) {
				s->c->pos -= s->c->right * s->c->speed * dT;
			}
			if (keys[SDL_SCANCODE_D]) {
				s->c->pos += s->c->right * s->c->speed * dT;
			}
		}
	}
	inputstate current;
	vector<callback> mouseup;
	vector<callback> mousedown;
	vector<callback> mousemotion;
	vector<callback> mousewheel;
	vector<callback> keydown;
	vector<callback> keyup;
	vector<callback> text;
};

void add_default_callbacks(state* s) {
	s->ev->mousemotion.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		float sens = 0.1f;
		float dx = (ev->motion.x - mx) * sens;
		float dy = (ev->motion.y - my) * sens;
		mx = ev->motion.x;
		my = ev->motion.y;
		if (s->ev->current == in_cam) {
			s->c->yaw += dx;
			s->c->pitch -= dy;
			if (s->c->yaw > 360.0f) s->c->yaw = 0.0f;
			else if (s->c->yaw < 0.0f) s->c->yaw = 360.0f;
			if (s->c->pitch > 89.0f) s->c->pitch = 89.0f;
			else if (s->c->pitch < -89.0f) s->c->pitch = -89.0f;
			s->c->updateFront();
		}
		return true;
	}, in_cam));

	s->ev->mousewheel.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		s->c->fov -= ev->wheel.y;
		if (s->c->fov > 179.0f) s->c->fov = 179.0f;
		else if (s->c->fov < 1.0f) s->c->fov = 1.0f;
		return true;
	}, in_cam));

	s->ev->mousedown.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		if (ev->button.x < (int)round(s->w * UI_SCREEN_RATIO) && ev->button.y > (s->ui->widgets.size() ? s->ui->widgets.back()->current_yh : 0)) {
			s->graphs.push_back(new graph(next_graph_id, "", -10, 10, -10, 10, 200, 200));
			s->graphs.back()->gen();
			s->ui->widgets.push_back(new fxy_equation(" ", next_graph_id, true));
			next_graph_id++;
			return true;
		}
		else {
			for (widget* w : s->ui->widgets) {
				if (ev->button.x < (int)round(s->w * UI_SCREEN_RATIO) && ev->button.y > w->current_y && ev->button.y <= w->current_yh) {
					w->active = true;
				}
			}
		}
		return false;
	}, in_ui));

	s->ev->mousedown.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		if (ev->button.x > (int)round(s->w * UI_SCREEN_RATIO) && ev->button.x <= (int)round(s->w * UI_SCREEN_RATIO) + 37 &&
			ev->button.y <= 32) {
			s->ui->active = false;
			s->ev->current = in_idle;
			return true;
		}
		return false;
	}, in_ui));

	s->ev->mousedown.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		if (ev->button.x > (int)round(s->w * UI_SCREEN_RATIO)) {
			s->ev->current = in_cam;
			SDL_CaptureMouse(SDL_TRUE);
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
		return false;
	}, in_ui));

	s->ev->mousedown.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		if (ev->button.x <= 43 && ev->button.y <= 32) {
			s->ui->active = true;
			s->ev->current = in_ui;
			return true;
		}
		return false;
	}, in_idle));

	s->ev->mousedown.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		if (ev->button.x > 43 && ev->button.y > 32) {
			s->ev->current = in_cam;
			SDL_CaptureMouse(SDL_TRUE);
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
		return false;
	}, in_idle));

	s->ev->mouseup.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		if (s->ui->active)
			s->ev->current = in_ui;
		else
			s->ev->current = in_idle;
		SDL_CaptureMouse(SDL_FALSE);
		SDL_SetRelativeMouseMode(SDL_FALSE);
		SDL_WarpMouseInWindow(s->window, s->ui->active ? (int)round(s->w * ((1 - UI_SCREEN_RATIO) / 2 + UI_SCREEN_RATIO)) : s->w / 2, s->h / 2);
		return true;
	}, in_cam));
}