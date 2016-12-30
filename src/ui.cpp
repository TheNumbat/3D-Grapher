
#include "ui.h"
#include "state.h"

#include <SDL_ttf.h>
#include <SDL.h>
#include <algorithm>
#include "data\tex_gear.data"
#include "data\tex_in.data"
#include "data\tex_out.data"
#include "data\tex_f.data"

// the code in this file is p disorganized, good luck

fxy_enter_callback::fxy_enter_callback(int i) {
	g_id = i;
}

void fxy_enter_callback::operator()(state* s, string e) const {
	if (e.size() && e != " ") {
		int g_ind = getIndex(s, g_id);
		if (g_ind == -1) {
			s->graphs.push_back(new graph(g_id, "", (float)s->set.xmin, (float)s->set.xmax, (float)s->set.ymin, (float)s->set.ymax, s->set.xrez, s->set.yrez));
			s->graphs.back()->gen();
			g_ind = s->graphs.size() - 1;
		}
		s->graphs[g_ind]->eq_str = e;
		regengraph(s, g_ind);
		s->ev.current = in_funcs;
		SDL_ShowCursor(1);
	}
}

fxy_remove_callback::fxy_remove_callback(int i) {
	g_id = i;
}

bool fxy_remove_callback::operator()(state* s) const {
	int g_ind = getIndex(s, g_id);
	if (g_ind >= 0) {
		delete s->graphs[g_ind];
		s->graphs.erase(s->graphs.begin() + g_ind);
		updateAxes(s);
	}
	return true;
}

UI::UI(state* s) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	active = false;
	uistate = ui_funcs;

	settings.push_back(new toggle_text("Wirefame", true, [](state* s) -> void {s->set.wireframe = !s->set.wireframe;}));
	settings.push_back(new toggle_text("Lighting", false, [](state* s) -> void {s->set.lighting = !s->set.lighting; }));
	settings.push_back(new toggle_text("Axis Normalization", false, [](state* s) -> void {s->set.axisnormalization = !s->set.axisnormalization; regenall(s); }));
	settings.push_back(new toggle_text("Antialiasing", true, [](state* s) -> void {
		if (s->set.antialiasing)
			glDisable(GL_MULTISAMPLE);
		else
			glEnable(GL_MULTISAMPLE);
		s->set.antialiasing = !s->set.antialiasing;
	}));

	vector<string> cameras = { "Camera: 2D", "Camera: 3D", "Camera: Fixed 3D" };
	settings.push_back(new multi_text(cameras, 2, [](state* s, string str) -> void {
		if (str == "Camera: 2D") {
			s->set.camtype = cam_2d;
		}
		else if (str == "Camera: 3D") {
			s->set.camtype = cam_3d;
		}
		else if (str == "Camera: Fixed 3D") {
			s->set.camtype = cam_3d_static;
		}
	}));

	settings.push_back(new slider("Opacity", 1.0f, [](state* s, float f) -> void {s->set.graphopacity = f; }));
	
	settings.push_back(new edit_text(s, to_string((int)s->set.xmin), "xmin: ", [](state* s, string exp) -> void {
		try {
			float num = stof(exp);
			if (num < s->set.xmax) {
				s->set.xmin = num;
				regenall(s);
			}
			else cout << "\terr: xmin >= xmax" << endl;
		}
		catch (...) {
			cout << "\terr: couldn't parse float" << endl;
		}
		s->ev.current = in_settings;
		SDL_ShowCursor(1);
	}, [](state* s) -> bool {return false; }, false));
	settings.push_back(new edit_text(s, to_string((int)s->set.xmax), "xmax: ", [](state* s, string exp) -> void {
		try {
			float num = stof(exp);
			if (num > s->set.xmin) {
				s->set.xmax = num;
				regenall(s);
			}
			else cout << "\terr: xmax <= xmin" << endl;
		}
		catch (...) {
			cout << "\terr: couldn't parse float" << endl;
		}
		s->ev.current = in_settings;
		SDL_ShowCursor(1);
	}, [](state* s) -> bool {return false; }, false));
	settings.push_back(new edit_text(s, to_string((int)s->set.ymin), "ymin: ", [](state* s, string exp) -> void {
		try {
			float num = stof(exp);
			if (num < s->set.ymax) {
				s->set.ymin = num;
				regenall(s);
			}
			else cout << "\terr: ymin >= ymax" << endl;
		}
		catch (...) {
			cout << "\terr: couldn't parse float" << endl;
		}
		s->ev.current = in_settings;
		SDL_ShowCursor(1);
	}, [](state* s) -> bool {return false; }, false));
	settings.push_back(new edit_text(s, to_string((int)s->set.ymax), "ymax: ", [](state* s, string exp) -> void {
		try {
			float num = stof(exp);
			if (num > s->set.ymin) {
				s->set.ymax = num;
				regenall(s);
			}
			else cout << "\terr: ymax <= ymin" << endl;
		}
		catch (...) {
			cout << "\terr: couldn't parse float" << endl;
		}
		s->ev.current = in_settings;
		SDL_ShowCursor(1);
	}, [](state* s) -> bool {return false; }, false));
	settings.push_back(new edit_text(s, to_string((int)s->set.xrez), "xrez: ", [](state* s, string exp) -> void {
		try {
			int num = stoi(exp);
			s->set.xrez = num;
			regenall(s);
		}
		catch (...) {
			cout << "\terr: couldn't parse int" << endl;
		}
		s->ev.current = in_settings;
		SDL_ShowCursor(1);
	}, [](state* s) -> bool {return false; }, false));
	settings.push_back(new edit_text(s, to_string((int)s->set.yrez), "yrez: ", [](state* s, string exp) -> void {
		try {
			int num = stoi(exp);
			s->set.yrez = num;
			regenall(s);
		}
		catch (...) {
			cout << "\terr: couldn't parse int" << endl;
		}
		s->ev.current = in_settings;
		SDL_ShowCursor(1);
	}, [](state* s) -> bool {return false; }, false));

	auto widgetsCallback = [](state* s, SDL_Event* ev) -> bool {
		if (s->ui->uistate == ui_funcs) {
			for (widget* w : s->ui->funcs) {
				if (w->update(s, ev)) return true;
			}
		}
		else {
			for (widget* w : s->ui->settings) {
				if (w->update(s, ev)) return true;
			}
		}
		return false;
	};
	s->ev.callbacks.push_back(callback(widgetsCallback, in_any, SDL_KEYDOWN));
	s->ev.callbacks.push_back(callback(widgetsCallback, in_any, SDL_TEXTINPUT));
	s->ev.callbacks.push_back(callback(widgetsCallback, in_any, SDL_WINDOWEVENT));
	s->ev.callbacks.push_back(callback(widgetsCallback, in_any, SDL_MOUSEBUTTONDOWN));
	s->ev.callbacks.push_back(callback(widgetsCallback, in_any, SDL_MOUSEBUTTONUP));
	s->ev.callbacks.push_back(callback(widgetsCallback, in_any, SDL_MOUSEMOTION));

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
			i--;
		}
	}
	for (unsigned int i = 0; i < settings.size(); i++) {
		if (settings[i]->should_remove) {
			delete settings[i];
			settings.erase(settings.begin() + i);
			i--;
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
	if (uistate == ui_funcs) {
		int cur_y = 0;
		unsigned int windex = 0;
		while (cur_y < h && windex < funcs.size()) {
			cur_y += 3;
			cur_y = funcs[windex]->render(s, w, h, ui_w, 5 + xoff, cur_y) + 3; // widget
			drawRect(s->rect_s, xoff, cur_y, ui_w, 3, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // top/bottom black strip
			cur_y += 3;
			windex++;
		}
	}
	else if(uistate == ui_settings) {
		int cur_y = 3;
		unsigned int windex = 0;
		while (cur_y < h && windex < settings.size()) {
			cur_y = settings[windex]->render(s, w, h, ui_w, 5 + xoff, cur_y) + 3;
			drawRect(s->rect_s, xoff, cur_y, ui_w, 3, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // top/bottom black strip
			cur_y += 3;
			windex++;
		}
	}
	drawRect(s->rect_s, xoff + ui_w, 0, 3, h, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // right black strip
	drawRect(s->rect_s, xoff, 0, ui_w, 3, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // top black strip
	drawRect(s->rect_s, xoff, h - 3, ui_w, 3, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // bottom black strip
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

edit_text::edit_text(state* s, string e, string h, function<void(state*, string)> c, function<bool(state*)> rm, bool a) {
	r.gen();
	active = a;
	exp = e;
	head = h;
	should_remove = false;
	cursor_x = 0;
	cursor_y = 0;
	cursor_pos = e == " " ? 0 : e.size();
	enterCallback = c;
	removeCallback = rm;
	break_str(s);
}

void edit_text::update_cursor(state* s) {
	break_str(s);
	TTF_SizeText(s->font, lines[currentLine()].substr(0, currentPos()).c_str(), &cursor_x, NULL);
}

bool edit_text::update(state* s, SDL_Event* ev) {
	if (ev->type == SDL_WINDOWEVENT) {
		break_str(s);
	}
	else if (active) {
		if (ev->type == SDL_TEXTINPUT) {
			if (exp == " ") exp.clear();
			exp.insert(cursor_pos, ev->text.text);
			cursor_pos++;
			return true;
		}
		else if (ev->type == SDL_KEYDOWN) {
			switch (ev->key.keysym.sym) {
			case SDLK_LEFT:
				if(cursor_pos > 0) cursor_pos--;
				return true;
			case SDLK_RIGHT:
				if (exp != " " && cursor_pos < (int)exp.size()) cursor_pos++;
				return true;
			case SDLK_DOWN:
				if (exp != " ") {
					cursor_pos += lines[currentLine()].size();
					if (cursor_pos > (int)exp.size()) cursor_pos = exp.size();
				}
				return true;
			case SDLK_UP:
				cursor_pos -= lines[currentLine()].size();
				if (cursor_pos < 0) cursor_pos = 0;
				return true;
			case SDLK_DELETE:
				if (exp != " ") {
					if (cursor_pos < (int)exp.size()) exp.erase(cursor_pos, 1);
					if (!exp.size()) exp = " ";
				}
				else remove(s);
				return true;
			case SDLK_BACKSPACE:
				if (exp != " ") {
					if (cursor_pos > 0) {
						exp.erase(cursor_pos - 1, 1);
						cursor_pos--;
					}
					if (!exp.size()) exp = " ";
				}
				else {
					active = false;
					remove(s);
				}
				return true;
			case SDLK_HOME:
				cursor_pos = 0;
				return true;
			case SDLK_END:
				cursor_pos = exp.size();
				return true;
			case SDLK_ESCAPE:
				active = false;
				SDL_StopTextInput();
				SDL_ShowCursor(1);
				if (s->ui->uistate == ui_funcs)
					s->ev.current = in_funcs;
				else
					s->ev.current = in_settings;
				return true;
			case SDLK_RETURN:
			case SDLK_RETURN2:
			case SDLK_KP_ENTER:
				active = false;
				enterCallback(s, exp);
				return true;
			}
		}
	}
	return false;
}

void edit_text::remove(state* s) {
	if (removeCallback(s)) {
		should_remove = true;
		s->ui->remove_dead_widgets(); // basically 'delete this' ... quesitonable practice
	}
	if (s->ui->uistate == ui_funcs)
		s->ev.current = in_funcs;
	else
		s->ev.current = in_settings;
	SDL_ShowCursor(1);
}

int edit_text::render(state* s, int w, int h, int ui_w, int x, int y) {
	current_y = y;
	for (string l : lines) {
		SDL_Surface* text = TTF_RenderText_Shaded(s->font, l.c_str(), { 0, 0, 0 }, { 255, 255, 255 });
		r.tex.load(text);
		r.set((float)x, (float)y, (float)text->w, (float)text->h);
		r.render(w, h, s->UI_s);
		y += text->h;
		SDL_FreeSurface(text);
	}
	if (active) {
		update_cursor(s);
		s->ui->drawRect(s->rect_s, x + cursor_x, current_y + 3 + cursor_y, 2, 24, 0.0f, 0.0f, 0.0f, 1.0f, (float)w, (float)h);
	}
	current_yh = y;
	return y;
}

int edit_text::currentPos() {
	int line = currentLine() - 1, cpos = cursor_pos;
	cpos += head.size();
	while (line >= 0) {
		cpos -= lines[line--].size();
	}
	return cpos;
}

int edit_text::currentLine() {
	int line = 0, cpos = cursor_pos;
	cpos += head.size();
	while (cpos >= 0 && line < (int)lines.size()) {
		cpos -= lines[line++].size();
	}
	return line - 1;
}

void edit_text::break_str(state* s) {
	lines.clear();
	string total = head + exp;
	int w = (int)round(s->w * UI_SCREEN_RATIO) - 5;
	int e_pos = 0, tw;
	TTF_SizeText(s->font, total.c_str(), &tw, NULL);
	do {
		unsigned int end = e_pos;
		int newtw;
		do {
			end++;
			TTF_SizeText(s->font, total.substr(e_pos, end - e_pos).c_str(), &newtw, NULL);
		} while (end < total.size() && newtw < w);
		if (newtw > w) {
			end--;
			TTF_SizeText(s->font, total.substr(e_pos, end - e_pos).c_str(), &newtw, NULL);
		}
		lines.push_back(total.substr(e_pos, end - e_pos));
		e_pos = end;
		tw -= newtw;
	} while (tw > 0);
	cursor_y = currentLine() * 29;
}

toggle_text::toggle_text(string t, bool o, function<void(state*)> c) {
	text = t;
	on = o;
	toggleCallback = c;
	should_remove = false;
	r.gen();
}

int toggle_text::render(state* s, int w, int h, int ui_w, int x, int y) {
	current_y = y;
	SDL_Color background;
	if (on) {
		background = { 191, 191, 191 };
		s->ui->drawRect(s->rect_s, x - 5, y, ui_w, 32, 0.0f, 0.0f, 0.0f, 0.25f, (float)w, (float)h);
	}
	else
		background = { 255, 255, 255 };
	SDL_Surface* surf = TTF_RenderText_Shaded(s->font, text.c_str(), { 0, 0, 0 }, background);
	r.tex.load(surf);
	r.set((float)x, (float)y, (float)(surf->w <= ui_w - 5 ? surf->w : ui_w - 5), (float)surf->h);
	r.render(w, h, s->UI_s);
	y += surf->h;
	SDL_FreeSurface(surf);
	current_yh = y;
	return y;
}

bool toggle_text::update(state* s, SDL_Event* ev) {
	if (ev->type == SDL_MOUSEBUTTONDOWN) {
		if (ev->button.y > current_y - 3 && ev->button.y <= current_yh + 3) {
			on = !on;
			toggleCallback(s);
			return true;
		}
	}
	return false;
}

multi_text::multi_text(vector<string> strs, int p, function<void(state*, string)> c) {
	text = strs;
	toggleCallback = c;
	should_remove = false;
	pos = p;
	r.gen();
}

int multi_text::render(state* s, int w, int h, int ui_w, int x, int y) {
	current_y = y;
	s->ui->drawRect(s->rect_s, x - 5, y, ui_w, 32, 0.0f, 0.0f, 0.0f, 0.25f, (float)w, (float)h);
	SDL_Surface* surf = TTF_RenderText_Shaded(s->font, text[pos].c_str(), { 0, 0, 0 }, { 191, 191, 191 });
	r.tex.load(surf);
	r.set((float)x, (float)y, (float)(surf->w <= ui_w - 5 ? surf->w : ui_w - 5), (float)surf->h);
	r.render(w, h, s->UI_s);
	y += surf->h;
	SDL_FreeSurface(surf);
	current_yh = y;
	return y;
}

bool multi_text::update(state* s, SDL_Event* ev) {
	if (ev->type == SDL_MOUSEBUTTONDOWN) {
		if (ev->button.y > current_y - 3 && ev->button.y <= current_yh + 3) {
			if (ev->button.button == SDL_BUTTON_LEFT)
				++pos %= text.size();
			else if (ev->button.button == SDL_BUTTON_RIGHT) {
				--pos;
				if (pos < 0) pos = text.size() - 1;
			}
			toggleCallback(s, text[pos]);
			return true;
		}
	}
	return false;
}

slider::slider(string t, float f, function<void(state*, float)> c) {
	text = t;
	moveCallback = c;
	pos = f;
	should_remove = false;
	slider_w = 0;
	r.gen();
}

int slider::render(state* s, int w, int h, int ui_w, int x, int y) {
	current_y = y;
	s->ui->drawRect(s->rect_s, x - 5, y, ui_w, 32, 0.0f, 0.0f, 0.0f, 0.25f, (float)w, (float)h);
	SDL_Surface* surf = TTF_RenderText_Shaded(s->font, text.c_str(), { 0, 0, 0 }, { 191, 191, 191 });
	r.tex.load(surf);
	int total_w = surf->w <= ui_w - 5 ? surf->w : ui_w - 25;
	r.set((float)x, (float)y, (float)total_w, (float)surf->h);
	r.render(w, h, s->UI_s);
	y += surf->h;
	current_yh = y;
	slider_w = ui_w - x - total_w - 6;
	s->ui->drawRect(s->rect_s, x + total_w + 3, ((current_yh - current_y) / 2) + current_y, slider_w, 2, 0.0f, 0.0f, 0.0f, 0.5f, (float)w, (float)h);
	s->ui->drawRect(s->rect_s, x + total_w + 3 + (int)round(pos * (slider_w - 10)), ((current_yh - current_y) / 2) + current_y - 4, 10, 10, 0.0f, 0.0f, 0.0f, 0.5f, (float)w, (float)h);
	SDL_FreeSurface(surf);
	return y;
}

bool slider::update(state* s, SDL_Event* ev) {
	if (active) {
		if (ev->type == SDL_MOUSEMOTION) {
			Uint32 mflags = SDL_GetMouseState(NULL, NULL);
			if (mflags & SDL_BUTTON(1) && slider_w) {
				pos += ev->motion.xrel / (float)slider_w;
				if (pos > 1) pos = 1;
				if (pos < 0) pos = 0;
				moveCallback(s, pos);
			}
			return true;
		}
		else if (ev->type == SDL_MOUSEBUTTONUP) {
			active = false;
			return true;
		}
	}
	else if (ev->type == SDL_MOUSEBUTTONDOWN && ev->button.y > current_y - 3 && ev->button.y <= current_yh + 3) {
		active = true;
		return true;
	}
	return false;
}