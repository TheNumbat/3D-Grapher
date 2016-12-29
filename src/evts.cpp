
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
		default:
			for (callback& c : callbacks) {
				if ((current == c.instate || c.instate == in_any) && ev.type == c.type) {
					if (c.func(s, &ev)) break;
				}
			}
			break;
		}
	}
	float dT = (SDL_GetTicks() - s->c_3d.lastUpdate) / 1000.0f;
	s->c_3d.lastUpdate = SDL_GetTicks();
	if (s->set.camtype == cam_3d && current == in_cam) {
		if (keys[SDL_SCANCODE_W]) {
			s->c_3d.pos += s->c_3d.front * s->c_3d.speed * dT;
		}
		if (keys[SDL_SCANCODE_S]) {
			s->c_3d.pos -= s->c_3d.front * s->c_3d.speed * dT;
		}
		if (keys[SDL_SCANCODE_A]) {
			s->c_3d.pos -= s->c_3d.right * s->c_3d.speed * dT;
		}
		if (keys[SDL_SCANCODE_D]) {
			s->c_3d.pos += s->c_3d.right * s->c_3d.speed * dT;
		}
	}
}

void add_default_callbacks(state* s) {
	s->ev.callbacks.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		int dx = (ev->motion.x - s->mx);
		int dy = (ev->motion.y - s->my);
		s->mx = ev->motion.x;
		s->my = ev->motion.y;
		if (s->set.camtype == cam_3d) {
			s->c_3d.move(dx, dy);
		}
		else if (s->set.camtype == cam_3d_static) {
			s->c_3d_static.move(dx, dy);
		}
		return true;
	}, in_cam, SDL_MOUSEMOTION));

	s->ev.callbacks.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		if (s->set.camtype == cam_3d) {
			s->c_3d.fov -= ev->wheel.y;
			if (s->c_3d.fov > 179.0f) s->c_3d.fov = 179.0f;
			else if (s->c_3d.fov < 1.0f) s->c_3d.fov = 1.0f;
		}
		else if (s->set.camtype == cam_3d_static) {
			s->c_3d_static.fov -= ev->wheel.y;
			if (s->c_3d_static.fov > 179.0f) s->c_3d_static.fov = 179.0f;
			else if (s->c_3d_static.fov < 1.0f) s->c_3d_static.fov = 1.0f;
		}
		return true;
	}, in_cam, SDL_MOUSEWHEEL));

	s->ev.callbacks.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		if (s->ui->uistate == ui_funcs) {
			if (ev->button.x < (int)round(s->w * UI_SCREEN_RATIO) && ev->button.y >(s->ui->funcs.size() ? s->ui->funcs.back()->current_yh : 0)) {
				s->graphs.push_back(new graph(s->next_graph_id, "", s->set.xmin, s->set.xmax, s->set.ymin, s->set.ymax, s->set.xrez, s->set.yrez));
				s->graphs.back()->gen();
				fxy_equation* w = new fxy_equation(s->next_graph_id, true);
				w->break_str(s, (int)round(s->w * UI_SCREEN_RATIO));
				s->ui->funcs.push_back(w);
				auto func = bind(&widget::update, w, placeholders::_1, placeholders::_2);
				s->ev.callbacks.push_back(callback(func, in_widget, SDL_TEXTINPUT, s->next_graph_id));
				s->ev.callbacks.push_back(callback(func, in_widget, SDL_KEYDOWN, s->next_graph_id));
				s->ev.callbacks.push_back(callback(func, in_any, SDL_WINDOWEVENT, s->next_graph_id));
				s->ev.current = in_widget;
				SDL_ShowCursor(0);
				s->next_graph_id++;
				SDL_StartTextInput();
				return true;
			}
			else {
				for (widget* w : s->ui->funcs) {
					if (ev->button.x < (int)round(s->w * UI_SCREEN_RATIO) && ev->button.y > w->current_y && ev->button.y <= w->current_yh) {
						w->active = true;
						SDL_StartTextInput();
						SDL_ShowCursor(0);
						s->ev.current = in_widget;
					}
				}
			}
		}
		return false;
	}, in_ui, SDL_MOUSEBUTTONDOWN));

	s->ev.callbacks.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		if (ev->button.x > (int)round(s->w * UI_SCREEN_RATIO)) {
			if (ev->button.x < (int)round(s->w * UI_SCREEN_RATIO) + 37) {
				if (ev->button.y >= 0 && ev->button.y <= 32) {
					s->ui->active = false;
					s->ev.current = in_idle;
					return true;
				}
				else if (ev->button.y >= 70 && ev->button.y <= 70 + 32) {
					if (s->ui->uistate == ui_funcs) {
						s->ui->uistate = ui_settings;
						s->ev.current = in_settings;
					}
					return true;
				}
			}
			s->ev.current = in_cam;
			SDL_CaptureMouse(SDL_TRUE);
			SDL_SetRelativeMouseMode(SDL_TRUE);
			s->mx = ev->button.x;
			s->my = ev->button.y;
			s->last_mx = ev->button.x;
			s->last_my = ev->button.y;
			return true;
		}
		return false;
	}, in_ui, SDL_MOUSEBUTTONDOWN));

	s->ev.callbacks.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		if (ev->button.x > (int)round(s->w * UI_SCREEN_RATIO)) {
			if (ev->button.x < (int)round(s->w * UI_SCREEN_RATIO) + 37) {
				if (ev->button.y >= 0 && ev->button.y <= 32) {
					s->ui->active = false;
					s->ev.current = in_idle;
					return true;
				}
				else if (ev->button.y >= 35 && ev->button.y <= 35 + 32) {
					if (s->ui->uistate == ui_settings) {
						s->ui->uistate = ui_funcs;
						s->ev.current = in_ui;
					}
					return true;
				}
			}
			s->ev.current = in_cam;
			SDL_CaptureMouse(SDL_TRUE);
			SDL_SetRelativeMouseMode(SDL_TRUE);
			s->mx = ev->button.x;
			s->my = ev->button.y;
			s->last_mx = ev->button.x;
			s->last_my = ev->button.y;
			return true;
		}
		return false;
	}, in_settings, SDL_MOUSEBUTTONDOWN));

	s->ev.callbacks.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		if (ev->button.x <= 43 && ev->button.y <= 32) {
			s->ui->active = true;
			if (s->ui->uistate == ui_funcs)
				s->ev.current = in_ui;
			else
				s->ev.current = in_settings;
			return true;
		}
		return false;
	}, in_idle, SDL_MOUSEBUTTONDOWN));

	s->ev.callbacks.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		if (ev->button.x > 43 || ev->button.y > 32) {
			s->ev.current = in_cam;
			s->mx = ev->button.x;
			s->my = ev->button.y;
			s->last_mx = ev->button.x;
			s->last_my = ev->button.y;
			SDL_CaptureMouse(SDL_TRUE);
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
		return false;
	}, in_idle, SDL_MOUSEBUTTONDOWN));

	s->ev.callbacks.push_back(callback([](state* s, SDL_Event* ev) -> bool {
		if (s->ui->active)
			if (s->ui->uistate == ui_funcs)
				s->ev.current = in_ui;
			else
				s->ev.current = in_settings;
		else
			s->ev.current = in_idle;
		SDL_CaptureMouse(SDL_FALSE);
		SDL_SetRelativeMouseMode(SDL_FALSE);
		SDL_WarpMouseInWindow(s->window, s->last_mx, s->last_my);
		return true;
	}, in_cam, SDL_MOUSEBUTTONUP));
}