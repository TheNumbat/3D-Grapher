
#include "ui.h"
#include "state.h"

#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <algorithm>
#include "tex_gear.data"
#include "tex_in.data"
#include "tex_out.data"
#include "tex_f.data"
#include "tex_q.data"

// the code in this file is p disorganized, good luck

graph_enter_callback::graph_enter_callback(int i, graph_type g) {
	g_id = i;
	gt = g;
}

void graph_enter_callback::operator()(state* s, string e) const {
	if (e.size() && e != " ") {
		int g_ind = getIndex(s, g_id);
		if (g_ind == -1) {
			switch (gt) {
			case graph_func:		s->graphs.push_back(new fxy_graph(g_id)); break;
			case graph_cylindrical: s->graphs.push_back(new cyl_graph(g_id)); break;
			}
			s->graphs.back()->gen();
			g_ind = s->graphs.size() - 1;
		}
		s->graphs[g_ind]->eq_str = e;
		s->ui->parseDoms(s);
		regengraph(s, g_ind);
		s->ev.current = in_funcs;
	}
}

graph_remove_callback::graph_remove_callback(int i) {
	g_id = i;
}

bool graph_remove_callback::operator()(state* s) const {
	int g_ind = getIndex(s, g_id);
	if (g_ind >= 0) {
		delete s->graphs[g_ind];
		s->graphs.erase(s->graphs.begin() + g_ind);
		updateAxes(s);
	}
	s->ev.current = in_funcs;
	return true;
}

UI::UI(state* s) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	active = true;
	uistate = ui_funcs;
	domain = graph_func;

	settings.push_back(new toggle_text("Wirefame", false, [](state* s) -> void {s->set.wireframe = !s->set.wireframe;}));
	settings.push_back(new toggle_text("Lighting", true, [](state* s) -> void {s->set.lighting = !s->set.lighting; }));
	settings.push_back(new toggle_text("Axis Normalization", false, [](state* s) -> void {s->set.axisnormalization = !s->set.axisnormalization; regenall(s); }));

	settings.push_back(new multi_text({ "Camera: Fixed 3D", "Camera: 3D" }, 0, [](state* s, string str) -> void {
		if (str == "Camera: 3D") {
			s->set.camtype = cam_3d;
		}
		else if (str == "Camera: Fixed 3D") {
			s->set.camtype = cam_3d_static;
		}
	}));

	settings.push_back(new slider("Opacity", 1.0f, [](state* s, float f) -> void {s->set.graphopacity = f; }));
	
	settings.push_back(new multi_text({ "Domain: Rectangluar", "Domain: Cylindrical" }, 0, [](state* s, string o) -> void {
		if (o == "Domain: Rectangluar") {
			s->ui->domain = graph_func;
		}
		else if(o == "Domain: Cylindrical") {
			s->ui->domain = graph_cylindrical;
		}
	}));

	auto domsCallback = [](state* s, string exp) -> void {
		if (exp.size() && exp != " ") {
			s->ui->parseDoms(s);
			regenall(s);
		}
		s->ev.current = in_settings;
	};
	auto domsRemoveCallback = [](state* s) -> bool {s->ev.current = in_settings; return false;};

#define add(val) dom_rect.push_back(new edit_text(s, to_string(s->set.rdom.val), string(#val) + ": ", domsCallback, domsRemoveCallback, false));
	add(xmin);
	add(xmax);
	add(ymin);
	add(ymax);
	add(zmin);
	add(zmax);
	add(xrez);
	add(yrez);
#undef add
#define add(val) dom_cyl.push_back(new edit_text(s, to_string(s->set.cdom.val), string(#val) + ": ", domsCallback, domsRemoveCallback, false));
	add(tmin);
	add(tmax);
	add(zmin);
	add(zmax);
	add(rmin);
	add(rmax);
	add(trez);
	add(zrez);
#undef add

	funcs_add.push_back(new static_text("f(x,y)", [](state* s) -> void {
		edit_text* w = new edit_text(s, " ", "f(x,y)= ", graph_enter_callback(s->next_graph_id, graph_func), graph_remove_callback(s->next_graph_id), true);
		s->next_graph_id++;
		s->ui->funcs.push_back(w);
		s->ev.current = in_widget;
		s->ui->uistate = ui_funcs;
		SDL_StartTextInput();
	}));

	funcs_add.push_back(new static_text("r(t,z)", [](state* s) -> void {
		edit_text* w = new edit_text(s, " ", "r(t,z)= ", graph_enter_callback(s->next_graph_id, graph_cylindrical), graph_remove_callback(s->next_graph_id), true);
		s->next_graph_id++;
		s->ui->funcs.push_back(w);
		s->ev.current = in_widget;
		s->ui->uistate = ui_funcs;
		SDL_StartTextInput();
	}));

	auto widgetsCallback = [](state* s, SDL_Event* ev) -> bool {
		if (s->ui->uistate == ui_funcs) {
			for (widget* w : s->ui->funcs) {
				if (w->update(s, ev)) return true;
			}
		}
		else if (s->ui->uistate == ui_settings) {
			for (widget* w : s->ui->settings) {
				if (w->update(s, ev)) return true;
			}
			if (s->ui->domain == graph_func) {
				for (widget* w : s->ui->dom_rect) {
					if (w->update(s, ev)) return true;
				}
			}
			else if (s->ui->domain == graph_cylindrical) {
				for (widget* w : s->ui->dom_cyl) {
					if (w->update(s, ev)) return true;
				}
			}
		}
		else {
			for (widget* w : s->ui->funcs_add) {
				if (w->update(s, ev)) return true;
			}
			if (ev->type == SDL_MOUSEBUTTONDOWN || (ev->type == SDL_KEYDOWN && ev->key.keysym.sym == SDLK_ESCAPE)) {
				s->ev.current = in_funcs;
				s->ui->uistate = ui_funcs;
				return true;
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

	in_r.gen();
	out_r.gen();
	gear_r.gen();
	f_r.gen();
	q_r.gen();
	in_r.tex.load(SDL_LoadBMP_RW(SDL_RWFromConstMem(in_bmp, in_bmp_len), 1));
	out_r.tex.load(SDL_LoadBMP_RW(SDL_RWFromConstMem(out_bmp, out_bmp_len), 1));
	gear_r.tex.load(SDL_LoadBMP_RW(SDL_RWFromConstMem(gear_bmp, gear_bmp_len), 1));
	f_r.tex.load(SDL_LoadBMP_RW(SDL_RWFromConstMem(f_bmp, f_bmp_len), 1));
	q_r.tex.load(SDL_LoadBMP_RW(SDL_RWFromConstMem(q_bmp, q_bmp_len), 1));
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

void UI::parseDoms(state* s) {
	for (widget* w : dom_rect) {
		edit_text* e = (edit_text*) w;
		vector<op> exp;
		in(e->exp, exp);
		float val = eval(exp);
		if (e->head == "xmin: ")
			s->set.rdom.xmin = val;
		else if (e->head == "xmax: ")
			s->set.rdom.xmax = val;
		else if (e->head == "ymin: ")
			s->set.rdom.ymin = val;
		else if (e->head == "ymax: ")
			s->set.rdom.ymax = val;
		else if (e->head == "zmin: ")
			s->set.rdom.zmin = val;
		else if (e->head == "zmax: ")
			s->set.rdom.zmax = val;
		else if (e->head == "xrez: ")
			s->set.rdom.xrez = (int)round(val);
		else if (e->head == "yrez: ")
			s->set.rdom.yrez = (int)round(val);
	}
	for (widget* w : dom_cyl) {
		edit_text* e = (edit_text*) w;
		vector<op> exp;
		in(e->exp, exp);
		float val = eval(exp);
		if (e->head == "tmin: ")
			s->set.cdom.tmin = val;
		else if (e->head == "tmax: ")
			s->set.cdom.tmax = val;
		else if (e->head == "zmin: ")
			s->set.cdom.zmin = val;
		else if (e->head == "zmax: ")
			s->set.cdom.zmax = val;
		else if (e->head == "rmin: ")
			s->set.cdom.rmin = val;
		else if (e->head == "rmax: ")
			s->set.cdom.rmax = val;
		else if (e->head == "trez: ")
			s->set.cdom.trez = (int)round(val);
		else if (e->head == "zrez: ")
			s->set.cdom.zrez = (int)round(val);
	}
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

void UI::render(state* s) {
	int ui_w = (int)round(s->w * UI_SCREEN_RATIO);
	int x;
	if (active)
		x = 0;
	else
		x = -ui_w + 5;

	int y = 6;

	if (uistate == ui_funcs || uistate == ui_funcs_adding) {
		render_widgets(s, funcs, ui_w, x, y, true);
	}
	else if (uistate == ui_settings) {
		y = render_widgets(s, settings, ui_w, x, y, true) - 3;
		if (domain == graph_func)
			render_widgets(s, dom_rect, ui_w, x, y, false);
		else if (domain == graph_cylindrical)
			render_widgets(s, dom_cyl, ui_w, x, y, false);
	}
	render_sidebar(s);
	if (uistate == ui_funcs_adding) {
		y = render_widgets(s, funcs_add, ui_w / 2, adding_x, adding_y, false);
		drawRect(s->rect_s, adding_x, adding_y, 3, y - adding_y, 0.0f, 0.0f, 0.0f, 1.0f, (float)s->w, (float)s->h); // right black strip
	}
}

int UI::render_widgets(state* s, vector<widget*>& v, int ui_w, int x, int y, bool fullborders) {
	if(fullborders) drawRect(s->rect_s, x, 0, ui_w, s->h, 1.0f, 1.0f, 1.0f, 1.0f, (float)s->w, (float)s->h); // white background
	int base_y = y;
	unsigned int windex = 0;
	while (y < s->h && windex < v.size()) {
		// 1st time this is rendered height will be zero
		drawRect(s->rect_s, x, y, ui_w, v[windex]->current_yh - v[windex]->current_y, 1.0f, 1.0f, 1.0f, 1.0f, (float)s->w, (float)s->h);

		y = v[windex]->render(s, ui_w, 5 + x, y);
		drawRect(s->rect_s, x, y, ui_w, 3, 0.0f, 0.0f, 0.0f, 1.0f, (float)s->w, (float)s->h); // top/bottom black strip
		y += 3;
		windex++;
	}
	drawRect(s->rect_s, x + ui_w, base_y, 3, (fullborders ? s->h : y) - base_y, 0.0f, 0.0f, 0.0f, 1.0f, (float)s->w, (float)s->h); // right black strip
	drawRect(s->rect_s, x, base_y, ui_w, 3, 0.0f, 0.0f, 0.0f, 1.0f, (float)s->w, (float)s->h); // top black strip
	drawRect(s->rect_s, x, (fullborders ? s->h : y) - 3, ui_w, 3, 0.0f, 0.0f, 0.0f, 1.0f, (float)s->w, (float)s->h); // bottom black strip
	return y;
}

void UI::render_sidebar(state* s) {
	if (active) {
		if (uistate == ui_funcs || uistate == ui_funcs_adding) {
			drawRect(s->rect_s, (int)round(s->w * UI_SCREEN_RATIO) + 3, 33, 36, 36, 0.0f, 0.0f, 0.0f, 0.251f, (float)s->w, (float)s->h);
		}
		else if (uistate == ui_settings) {
			drawRect(s->rect_s, (int)round(s->w * UI_SCREEN_RATIO) + 3, 68, 36, 36, 0.0f, 0.0f, 0.0f, 0.251f, (float)s->w, (float)s->h);
		}
		in_r.set((int)round(s->w * UI_SCREEN_RATIO) + 5.0f, 0, 32, 32);
		in_r.render(s->w, s->h, s->UI_s);
		f_r.set((int)round(s->w * UI_SCREEN_RATIO) + 5.0f, 35, 32, 32);
		f_r.render(s->w, s->h, s->UI_s);
		gear_r.set((int)round(s->w * UI_SCREEN_RATIO) + 5.0f, 70, 32, 32);
		gear_r.render(s->w, s->h, s->UI_s);
		q_r.set((int)round(s->w * UI_SCREEN_RATIO) + 5.0f, 105, 32, 32);
		q_r.render(s->w, s->h, s->UI_s);
	}
	else {
		out_r.set(11, 0, 32, 32);
		out_r.render(s->w, s->h, s->UI_s);
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
	if (exp == " ") {
		exp = "";
		cursor_pos = 0;
	}
	break_str(s);
	TTF_SizeText(s->font, lines[currentLine()].substr(0, currentPos()).c_str(), &cursor_x, NULL);
	if (exp == "") exp = " ";
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
				if (cursor_pos > 0) cursor_pos--;
				return true;
			case SDLK_RIGHT:
				if (exp != " " && cursor_pos < (int)exp.size()) cursor_pos++;
				return true;
			case SDLK_DOWN:
				if (exp != " ") {
					cursor_pos += lines[currentLine()].size();
					if (cursor_pos >(int)exp.size()) cursor_pos = exp.size();
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
	if (ev->type == SDL_MOUSEBUTTONDOWN) {
		if (!active && ev->button.x < (int)round(s->w * UI_SCREEN_RATIO) && ev->button.y > current_y && ev->button.y <= current_yh
			&& ev->button.x >= current_x && ev->button.x <= current_xw && s->ev.current != in_widget) {
			active = true;
			SDL_StartTextInput();
			s->ev.current = in_widget;
			return true;
		}
		else if(active) {
			active = false;
			SDL_StopTextInput();
			if (s->ui->active)
				s->ev.current = in_settings;
			else
				s->ev.current = in_funcs;
			return true;
		}
	}
	return false;
}

void edit_text::remove(state* s) {
	if (removeCallback(s)) {
		should_remove = true;
		s->ui->remove_dead_widgets(); // basically 'delete this' ... quesitonable practice
	}
	SDL_ShowCursor(1);
}

int edit_text::render(state* s, int ui_w, int x, int y) {
	current_y = y;
	current_x = x;
	for (string l : lines) {
		SDL_Surface* text = TTF_RenderUTF8_Shaded(s->font, l.c_str(), { 0, 0, 0 }, { 255, 255, 255 });
		r.tex.load(text);
		r.set((float)x, (float)y, (float)text->w, (float)text->h);
		r.render(s->w, s->h, s->UI_s);
		y += text->h;
		SDL_FreeSurface(text);
	}
	if (active) {
		update_cursor(s);
		s->ui->drawRect(s->rect_s, x + cursor_x, current_y + 3 + cursor_y, 2, 24, 0.0f, 0.0f, 0.0f, 1.0f, (float)s->w, (float)s->h);
	}
	current_yh = y;
	current_xw = x + ui_w;
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
		if (end == e_pos) break;
		lines.push_back(total.substr(e_pos, end - e_pos));
		e_pos = end;
		tw -= newtw;
	} while (tw > 0);
	cursor_y = currentLine() * 29;
}

static_text::static_text(string t, function<void(state*)> c) {
	text = t;
	clickCallback = c;
	r.gen();
}

int static_text::render(state* s, int ui_w, int x, int y) {
	current_y = y;
	current_x = x;
	SDL_Surface* surf = TTF_RenderUTF8_Shaded(s->font, text.c_str(), { 0, 0, 0 }, { 255, 255, 255 });
	r.tex.load(surf);
	r.set((float)x, (float)y, (float)(surf->w <= ui_w - 5 ? surf->w : ui_w - 5), (float)surf->h);
	r.render(s->w, s->h, s->UI_s);
	y += surf->h;
	SDL_FreeSurface(surf);
	current_yh = y;
	current_xw = x + ui_w;
	return y;
}

bool static_text::update(state* s, SDL_Event* ev) {
	if (ev->type == SDL_MOUSEBUTTONDOWN && s->ev.current != in_widget) {
		if (ev->button.y > current_y - 3 && ev->button.y <= current_yh + 3
			&& ev->button.x >= current_x && ev->button.x <= current_xw) {
			clickCallback(s);
			return true;
		}
	}
	return false;
}

toggle_text::toggle_text(string t, bool o, function<void(state*)> c) {
	text = t;
	on = o;
	toggleCallback = c;
	should_remove = false;
	r.gen();
}

int toggle_text::render(state* s, int ui_w, int x, int y) {
	current_y = y;
	current_x = x;
	SDL_Color background;
	if (on) {
		background = { 191, 191, 191 };
		s->ui->drawRect(s->rect_s, x - 5, y, ui_w, 32, 0.0f, 0.0f, 0.0f, 0.25f, (float)s->w, (float)s->h);
	}
	else
		background = { 255, 255, 255 };
	SDL_Surface* surf = TTF_RenderUTF8_Shaded(s->font, text.c_str(), { 0, 0, 0 }, background);
	r.tex.load(surf);
	r.set((float)x, (float)y, (float)(surf->w <= ui_w - 5 ? surf->w : ui_w - 5), (float)surf->h);
	r.render(s->w, s->h, s->UI_s);
	y += surf->h;
	SDL_FreeSurface(surf);
	current_yh = y;
	current_xw = x + ui_w;
	return y;
}

bool toggle_text::update(state* s, SDL_Event* ev) {
	if (ev->type == SDL_MOUSEBUTTONDOWN && s->ev.current != in_widget) {
		if (ev->button.y > current_y - 3 && ev->button.y <= current_yh + 3
			&& ev->button.x >= current_x && ev->button.x <= current_xw) {
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

int multi_text::render(state* s, int ui_w, int x, int y) {
	current_y = y;
	current_x = x;
	s->ui->drawRect(s->rect_s, x - 5, y, ui_w, 32, 0.0f, 0.0f, 0.0f, 0.25f, (float)s->w, (float)s->h);
	SDL_Surface* surf = TTF_RenderUTF8_Shaded(s->font, text[pos].c_str(), { 0, 0, 0 }, { 191, 191, 191 });
	r.tex.load(surf);
	r.set((float)x, (float)y, (float)(surf->w <= ui_w - 5 ? surf->w : ui_w - 5), (float)surf->h);
	r.render(s->w, s->h, s->UI_s);
	y += surf->h;
	SDL_FreeSurface(surf);
	current_yh = y;
	current_xw = x + ui_w;
	return y;
}

bool multi_text::update(state* s, SDL_Event* ev) {
	if (ev->type == SDL_MOUSEBUTTONDOWN && s->ev.current != in_widget) {
		if (ev->button.y > current_y - 3 && ev->button.y <= current_yh + 3
			&& ev->button.x >= current_x && ev->button.x <= current_xw) {
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

int slider::render(state* s, int ui_w, int x, int y) {
	current_y = y;
	current_x = x;
	s->ui->drawRect(s->rect_s, x - 5, y, ui_w, 32, 0.0f, 0.0f, 0.0f, 0.25f, (float)s->w, (float)s->h);
	SDL_Surface* surf = TTF_RenderUTF8_Shaded(s->font, text.c_str(), { 0, 0, 0 }, { 191, 191, 191 });
	r.tex.load(surf);
	int total_w = surf->w <= ui_w - 5 ? surf->w : ui_w - 25;
	r.set((float)x, (float)y, (float)total_w, (float)surf->h);
	r.render(s->w, s->h, s->UI_s);
	y += surf->h;
	current_yh = y;
	current_xw = x + ui_w;
	slider_w = ui_w - total_w - 11;
	s->ui->drawRect(s->rect_s, x + total_w + 3, ((current_yh - current_y) / 2) + current_y, slider_w, 2, 0.0f, 0.0f, 0.0f, 0.5f, (float)s->w, (float)s->h);
	s->ui->drawRect(s->rect_s, x + total_w + 3 + (int)round(pos * (slider_w - 10)), ((current_yh - current_y) / 2) + current_y - 4, 10, 10, 0.0f, 0.0f, 0.0f, 0.5f, (float)s->w, (float)s->h);
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
	else if (ev->type == SDL_MOUSEBUTTONDOWN && ev->button.y > current_y - 3 && ev->button.y <= current_yh + 3
			 && ev->button.x >= current_x && ev->button.x <= current_xw && s->ev.current != in_widget) {
		active = true;
		return true;
	}
	return false;
}