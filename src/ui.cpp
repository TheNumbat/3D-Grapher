
#include "ui.h"
#include "state.h"

#include <SDL_ttf.h>
#include <SDL.h>
#include <algorithm>
#include "data\tex_gear.data"
#include "data\tex_in.data"
#include "data\tex_out.data"
#include "data\tex_f.data"

// the code in this file is p bad and disorganized, good luck

UI::UI(state* s) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	active = false;
	uistate = ui_funcs;

	settings.push_back(new toggle_text("Wirefame", true, [](state* s) -> void {s->set.wireframe = !s->set.wireframe;}));
	settings.push_back(new toggle_text("Lighting", false, [](state* s) -> void {s->set.lighting = !s->set.lighting; }));
	settings.push_back(new toggle_text("Axis Normalization", false, [](state* s) -> void {s->set.axisnormalization = !s->set.axisnormalization; regenall(s); }));

	vector<string> cameras = { "Camera: 2D", "Camera: Static 2D", "Camera: 3D", "Camera: Static 3D" };
	settings.push_back(new multi_text(cameras, 2, [](state* s, string str) -> void {
		if (str == "Camera: 2D") {
			s->set.camtype = cam_2d;
		}
		else if (str == "Camera: Static 2D") {
			s->set.camtype = cam_2d_static;
		}
		else if (str == "Camera: 3D") {
			s->set.camtype = cam_3d;
		}
		else if (str == "Camera: Static 3D") {
			s->set.camtype = cam_3d_static;
		}
	}));
	
	auto settingsCallback = [](state* s, SDL_Event* ev) -> bool {
		if (ev->type == SDL_MOUSEBUTTONDOWN) {
			if (ev->button.x < (int)round(s->w * UI_SCREEN_RATIO)) {
				for (widget* w : s->ui->settings) {
					if (ev->button.y > w->current_y - 3 && ev->button.y <= w->current_yh + 3) {
						w->update(s, ev);
						return true;
					}
				}
			}
		}
		return false;
	};
	s->ev.callbacks.push_back(callback(settingsCallback, in_settings, SDL_MOUSEBUTTONDOWN));
	s->ev.callbacks.push_back(callback(settingsCallback, in_settings, SDL_MOUSEMOTION));

	in.gen();
	out.gen();
	gear.gen();
	f.gen();
	in.tex.load(SDL_LoadBMP_RW(SDL_RWFromConstMem(in_bmp, in_bmp_len), 1));
	out.tex.load(SDL_LoadBMP_RW(SDL_RWFromConstMem(out_bmp, out_bmp_len), 1));
	gear.tex.load(SDL_LoadBMP_RW(SDL_RWFromConstMem(gear_bmp, gear_bmp_len), 1));
	f.tex.load(SDL_LoadBMP_RW(SDL_RWFromConstMem(f_bmp, f_bmp_len), 1));
}

UI::~UI() {
	for (widget* w : funcs)
		delete w;
	for (widget* w : settings)
		delete w;
	funcs.clear();
	settings.clear();
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void UI::remove_dead_widgets() {
	for (unsigned int i = 0; i < funcs.size(); i++) {
		if (funcs[i]->should_remove) {
			delete funcs[i];
			funcs.erase(funcs.begin() + i);
		}
	}
	for (unsigned int i = 0; i < settings.size(); i++) {
		if (settings[i]->should_remove) {
			delete settings[i];
			settings.erase(settings.begin() + i);
		}
	}
}

void UI::drawRect(shader& shader, int x, int y, int w, int h, float r, float g, float b, float a, float screenw, float screenh) {
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

void UI::render(state* s, int w, int h) {
	float fw = (float)w, fh = (float)h;
	int xoff, ui_w = (int)round(w * UI_SCREEN_RATIO);
	if (active)
		xoff = 0;
	else
		xoff = -ui_w + 5;
	drawRect(s->rect_s, xoff, 0, ui_w, h, 1.0f, 1.0f, 1.0f, 1.0f, fw, fh); // white background
	drawRect(s->rect_s, xoff + ui_w, 0, 3, h, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // right black strip
	drawRect(s->rect_s, xoff, 0, ui_w, 3, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // top black strip
	drawRect(s->rect_s, xoff, h - 3, ui_w, 3, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // bottom black strip
	if (uistate == ui_funcs) {
		int cur_y = 0;
		unsigned int windex = 0;
		while (cur_y < h && windex < funcs.size()) {
			cur_y += 3;
			int w_y = funcs[windex]->render(s, w, h, ui_w, 5 + xoff, cur_y); // widget
			if(funcs[windex]->active)
				drawRect(s->rect_s, xoff + 5 + funcs[windex]->cursor_x, cur_y + 3 + funcs[windex]->cursor_y, 2, 24, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // cursor
			cur_y = w_y + 3;
			drawRect(s->rect_s, xoff, cur_y, ui_w, 3, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // top/bottom black strip
			cur_y += 3;
			windex++;
		}
	}
	else if(uistate == ui_settings) {
		int cur_y = 3;
		unsigned int windex = 0;
		while (cur_y < h && windex < settings.size()) {
			if (toggle_text* t = dynamic_cast<toggle_text*>(settings[windex])) {
				if (t->on) {
					drawRect(s->rect_s, xoff, cur_y, ui_w, 32, 0.0f, 0.0f, 0.0f, 0.25f, fw, fh);
				}
			}
			else {
				drawRect(s->rect_s, xoff, cur_y, ui_w, 32, 0.0f, 0.0f, 0.0f, 0.25f, fw, fh);
			}
			cur_y = settings[windex]->render(s, w, h, ui_w, 5 + xoff, cur_y) + 3;
			drawRect(s->rect_s, xoff, cur_y, ui_w, 3, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // top/bottom black strip
			cur_y += 3;
			windex++;
		}
	}
	render_sidebar(s);
}

void UI::render_sidebar(state* s) {
	if (active) {
		if (uistate == ui_funcs) {
			drawRect(s->rect_s, (int)round(s->w * UI_SCREEN_RATIO) + 3, 33, 36, 36, 0.0f, 0.0f, 0.0f, 0.251f, (float)s->w, (float)s->h);
		}
		else if (uistate == ui_settings) {
			drawRect(s->rect_s, (int)round(s->w * UI_SCREEN_RATIO) + 3, 68, 36, 36, 0.0f, 0.0f, 0.0f, 0.251f, (float)s->w, (float)s->h);
		}
		in.set((int)round(s->w * UI_SCREEN_RATIO) + 5.0f, 0, 32, 32);
		in.render(s->w, s->h, s->UI_s);
		f.set((int)round(s->w * UI_SCREEN_RATIO) + 5.0f, 35, 32, 32);
		f.render(s->w, s->h, s->UI_s);
		gear.set((int)round(s->w * UI_SCREEN_RATIO) + 5.0f, 70, 32, 32);
		gear.render(s->w, s->h, s->UI_s);
	}
	else {
		out.set(11, 0, 32, 32);
		out.render(s->w, s->h, s->UI_s);
	}
}

fxy_equation::fxy_equation(int graph_id, bool a) {
	r.gen();
	g_id = graph_id;
	active = a;
	exp = " ";
	should_remove = false;
	cursor_x = 0;
	cursor_y = 0;
	cursor_pos = 0;
}

void fxy_equation::update_cursor(state* s) {
	break_str(s, (int)round(s->w * UI_SCREEN_RATIO) - 5);
	TTF_SizeText(s->font, lines[currentLine()].substr(0, currentPos()).c_str(), &cursor_x, NULL);
}

bool fxy_equation::update(state* s, SDL_Event* ev) {
	if (active) {
		if (ev->type == SDL_TEXTINPUT) {
			if (exp == " ") exp.clear();
			exp.insert(cursor_pos, ev->text.text);
			cursor_pos++;
			update_cursor(s);
			return true;
		}
		if (ev->type == SDL_KEYDOWN) {
			int g_ind = getIndex(s, g_id);
			switch (ev->key.keysym.sym) {
			case SDLK_LEFT:
				if(cursor_pos > 0) cursor_pos--;
				update_cursor(s);
				return true;
			case SDLK_RIGHT:
				if (cursor_pos < (int)exp.size()) cursor_pos++;
				update_cursor(s);
				return true;
			case SDLK_DOWN:
				cursor_pos += lines[currentLine()].size();
				if (cursor_pos > (int)exp.size()) cursor_pos = exp.size();
				update_cursor(s);
				return true;
			case SDLK_UP:
				cursor_pos -= lines[currentLine()].size();
				if (cursor_pos < 0) cursor_pos = 0;
				update_cursor(s);
				return true;
			case SDLK_DELETE:
				if (exp != " ") {
					if (cursor_pos < (int)exp.size()) exp.erase(cursor_pos, 1);
					if (!exp.size()) exp = " ";
					update_cursor(s);
				}
				else remove(s);
				return true;
			case SDLK_BACKSPACE:
				if (exp != " ") {
					exp.erase(cursor_pos - 1, 1);
					cursor_pos--;
					if (!exp.size()) exp = " ";
					update_cursor(s);
				}
				else remove(s);
				return true;
			case SDLK_HOME:
				cursor_pos = 0;
				update_cursor(s);
				return true;
			case SDLK_END:
				cursor_pos = exp.size();
				update_cursor(s);
				return true;
			case SDLK_ESCAPE:
				active = false;
				SDL_StopTextInput();
				s->ev.current = in_ui;
				SDL_ShowCursor(1);
				return true;
			case SDLK_RETURN:
			case SDLK_RETURN2:
			case SDLK_KP_ENTER:
				if (exp != " ") {
					active = false;
					s->graphs[g_ind]->eq_str = exp;
					regengraph(s, g_ind);
					s->ev.current = in_ui;
					SDL_ShowCursor(1);
				}
				return true;
			}
		}
	}
	return false;
}

void fxy_equation::remove(state* s) {
	int g_ind = getIndex(s, g_id);
	delete s->graphs[g_ind];
	s->graphs.erase(s->graphs.begin() + g_ind);
	should_remove = true;
	active = false;
	s->ev.callbacks.erase(remove_if(s->ev.callbacks.begin(), s->ev.callbacks.end(), [this](callback& c) -> bool { return c.id == g_id; }), s->ev.callbacks.end());
	s->ui->remove_dead_widgets(); // basically 'delete this' ... quesitonable practice
	s->ev.current = in_ui;
	SDL_ShowCursor(1);
}

int fxy_equation::render(state* s, int w, int h, int ui_w, int x, int y) {
	current_y = y;
	for (string l : lines) {
		SDL_Surface* text = TTF_RenderText_Shaded(s->font, l.c_str(), { 0, 0, 0 }, { 255, 255, 255 });
		r.tex.load(text);
		r.set((float)x, (float)y, (float)text->w, (float)text->h);
		r.render(w, h, s->UI_s);
		y += text->h;
		SDL_FreeSurface(text);
	}
	current_yh = y;
	return y;
}

int fxy_equation::currentPos() {
	int line = currentLine() - 1, cpos = cursor_pos;
	while (line >= 0) {
		cpos -= lines[line--].size();
	}
	return cpos;
}

int fxy_equation::currentLine() {
	int line = 0, cpos = cursor_pos;
	while (cpos >= 0 && line < (int)lines.size()) {
		cpos -= lines[line++].size();
	}
	return line - 1;
}

void fxy_equation::break_str(state* s, int w) {
	lines.clear();
	int e_pos = 0, tw;
	TTF_SizeText(s->font, exp.c_str(), &tw, NULL);
	do {
		unsigned int end = e_pos;
		int newtw;
		do {
			end++;
			TTF_SizeText(s->font, exp.substr(e_pos, end - e_pos).c_str(), &newtw, NULL);
		} while (end < exp.size() && newtw < w);
		if (newtw > w) {
			end--;
			TTF_SizeText(s->font, exp.substr(e_pos, end - e_pos).c_str(), &newtw, NULL);
		}
		lines.push_back(exp.substr(e_pos, end - e_pos));
		e_pos = end;
		tw -= newtw;
	} while (tw > 0);
	cursor_y = currentLine() * 29;
}

toggle_text::toggle_text(string t, bool o, function<void(state*)> c) {
	text = t;
	on = o;
	toggleCallback = c;
	r.gen();
}

int toggle_text::render(state* s, int w, int h, int ui_w, int x, int y) {
	current_y = y;
	SDL_Color background;
	if (on)
		background = { 191, 191, 191 };
	else
		background = { 255, 255, 255 };
	SDL_Surface* surf = TTF_RenderText_Shaded(s->font, text.c_str(), { 0, 0, 0 }, background);
	r.tex.load(surf);
	r.set((float)x, (float)y, (float)surf->w, (float)surf->h);
	r.render(w, h, s->UI_s);
	y += surf->h;
	SDL_FreeSurface(surf);
	current_yh = y;
	return y;
}

bool toggle_text::update(state* s, SDL_Event* ev) {
	on = !on;
	toggleCallback(s);
	return true;
}

multi_text::multi_text(vector<string> strs, int p, function<void(state*, string)> c) {
	text = strs;
	toggleCallback = c;
	pos = p;
	r.gen();
}

int multi_text::render(state* s, int w, int h, int ui_w, int x, int y) {
	current_y = y;
	SDL_Surface* surf = TTF_RenderText_Shaded(s->font, text[pos].c_str(), { 0, 0, 0 }, { 191, 191, 191 });
	r.tex.load(surf);
	r.set((float)x, (float)y, (float)surf->w, (float)surf->h);
	r.render(w, h, s->UI_s);
	y += surf->h;
	SDL_FreeSurface(surf);
	current_yh = y;
	return y;
}

bool multi_text::update(state* s, SDL_Event* ev) {
	++pos %= text.size();
	toggleCallback(s, text[pos]);
	return true;
}