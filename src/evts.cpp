
#include "evts.h"
#include "gl.h"
#include "state.h"

callback::callback(function<bool(state*, SDL_Event*)> f, inputstate s, SDL_EventType t, int i) {
	func = f;
	instate = s;
	type = t;
	id = i;
}

evts::evts() {
	current = in_idle;
}

void evts::run(state* s) {
	const static unsigned char* keys = SDL_GetKeyboardState(NULL);
	SDL_Event ev;
	while (SDL_PollEvent(&ev) != 0) {
		Uint8 flags = 0;
		switch (ev.type) {
		case SDL_QUIT:
			s->running = false;
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
		default:
			for (callback& c : callbacks) {
				if (current == c.instate && ev.type == c.type) {
					if (c.func(s, &ev)) break;
				}
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

void add_default_callbacks(state* s) {
	s->ev.callbacks.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		float sens = 0.1f;
		float dx = (ev->motion.x - mx) * sens;
		float dy = (ev->motion.y - my) * sens;
		mx = ev->motion.x;
		my = ev->motion.y;
		if (s->ev.current == in_cam) {
			s->c.yaw += dx;
			s->c.pitch -= dy;
			if (s->c.yaw > 360.0f) s->c.yaw = 0.0f;
			else if (s->c.yaw < 0.0f) s->c.yaw = 360.0f;
			if (s->c.pitch > 89.0f) s->c.pitch = 89.0f;
			else if (s->c.pitch < -89.0f) s->c.pitch = -89.0f;
			s->c.updateFront();
		}
		return true;
	}, in_cam, SDL_MOUSEMOTION));

	s->ev.callbacks.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		s->c.fov -= ev->wheel.y;
		if (s->c.fov > 179.0f) s->c.fov = 179.0f;
		else if (s->c.fov < 1.0f) s->c.fov = 1.0f;
		return true;
	}, in_cam, SDL_MOUSEWHEEL));

	s->ev.callbacks.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		if (ev->button.x < (int)round(s->w * UI_SCREEN_RATIO) && ev->button.y > (s->ui->widgets.size() ? s->ui->widgets.back()->current_yh : 0)) {
			s->graphs.push_back(new graph(next_graph_id, "", -10, 10, -10, 10, 200, 200));
			s->graphs.back()->gen();
			widget* w = new fxy_equation(next_graph_id, true);
			s->ui->widgets.push_back(w);
			auto func = bind(&widget::update, w, placeholders::_1, placeholders::_2);
			s->ev.callbacks.push_back(callback(func, in_widget, SDL_TEXTINPUT, next_graph_id));
			s->ev.callbacks.push_back(callback(func, in_widget, SDL_KEYDOWN, next_graph_id));
			s->ev.current = in_widget;
			SDL_ShowCursor(0);
			next_graph_id++;
			return true;
		}
		else {
			for (widget* w : s->ui->widgets) {
				if (ev->button.x < (int)round(s->w * UI_SCREEN_RATIO) && ev->button.y > w->current_y && ev->button.y <= w->current_yh) {
					w->active = true;
					SDL_ShowCursor(0);
					SDL_StartTextInput();
					s->ev.current = in_widget;
				}
			}
		}
		return false;
	}, in_ui, SDL_MOUSEBUTTONDOWN));

	s->ev.callbacks.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		if (ev->button.x > (int)round(s->w * UI_SCREEN_RATIO) && ev->button.x <= (int)round(s->w * UI_SCREEN_RATIO) + 37 &&
			ev->button.y <= 32) {
			s->ui->active = false;
			s->ev.current = in_idle;
			return true;
		}
		return false;
	}, in_ui, SDL_MOUSEBUTTONDOWN));

	s->ev.callbacks.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		if (ev->button.x > (int)round(s->w * UI_SCREEN_RATIO)) {
			s->ev.current = in_cam;
			SDL_CaptureMouse(SDL_TRUE);
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
		return false;
	}, in_ui, SDL_MOUSEBUTTONDOWN));

	s->ev.callbacks.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		if (ev->button.x <= 43 && ev->button.y <= 32) {
			s->ui->active = true;
			s->ev.current = in_ui;
			return true;
		}
		return false;
	}, in_idle, SDL_MOUSEBUTTONDOWN));

	s->ev.callbacks.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		if (ev->button.x > 43 && ev->button.y > 32) {
			s->ev.current = in_cam;
			SDL_CaptureMouse(SDL_TRUE);
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
		return false;
	}, in_idle, SDL_MOUSEBUTTONDOWN));

	s->ev.callbacks.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		if (s->ui->active)
			s->ev.current = in_ui;
		else
			s->ev.current = in_idle;
		SDL_CaptureMouse(SDL_FALSE);
		SDL_SetRelativeMouseMode(SDL_FALSE);
		SDL_WarpMouseInWindow(s->window, s->ui->active ? (int)round(s->w * ((1 - UI_SCREEN_RATIO) / 2 + UI_SCREEN_RATIO)) : s->w / 2, s->h / 2);
		return true;
	}, in_cam, SDL_MOUSEBUTTONUP));
}