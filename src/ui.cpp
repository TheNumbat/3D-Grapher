
#include "ui.h"
#include "state.h"

#include <SDL_ttf.h>
#include <SDL.h>
#include <algorithm>
#include "data\tex_gear.data"
#include "data\tex_in.data"
#include "data\tex_out.data"

UI::UI() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	active = false;
	in.gen();
	out.gen();
	gear.gen();
	in.tex.load(SDL_LoadBMP_RW(SDL_RWFromConstMem(in_bmp, in_bmp_len), 1));
	out.tex.load(SDL_LoadBMP_RW(SDL_RWFromConstMem(out_bmp, out_bmp_len), 1));
	gear.tex.load(SDL_LoadBMP_RW(SDL_RWFromConstMem(gear_bmp, gear_bmp_len), 1));
}

UI::~UI() {
	for (widget* w : widgets)
		delete w;
	widgets.clear();
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void UI::remove_dead_widgets() {
	for (unsigned int i = 0; i < widgets.size(); i++) {
		if (widgets[i]->should_remove) {
			delete widgets[i];
			widgets.erase(widgets.begin() + i);
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

void UI::render(state* s, int w, int h, shader& ui_s, shader& rect_s) {
	float fw = (float)w, fh = (float)h;
	int xoff, ui_w = (int)round(w * UI_SCREEN_RATIO);
	if (active)
		xoff = 0;
	else
		xoff = -ui_w + 5;
	drawRect(rect_s, xoff, 0, ui_w, h, 1.0f, 1.0f, 1.0f, 1.0f, fw, fh); // white background
	drawRect(rect_s, xoff + ui_w, 0, 3, h, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // right black strip
	int cur_y = 0;
	unsigned int windex = 0;
	while (cur_y < h && windex < widgets.size()) {
		drawRect(rect_s, xoff, cur_y, ui_w, 3, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // top/bottom black strip
		cur_y += 3;
		cur_y = widgets[windex++]->render(s, w, h, ui_w, 5 + xoff, cur_y, ui_s); // widget
		cur_y += 3;
	}
	drawRect(rect_s, xoff, cur_y, ui_w, 3, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // bottom black strip
	if (active) {
		in.set(ui_w + 5.0f, 0, 32, 32);
		in.render(w, h, ui_s);
		gear.set(ui_w + 5.0f, 35, 32, 32);
		gear.render(w, h, ui_s);
	}
	else {
		out.set(11, 0, 32, 32);
		out.render(w, h, ui_s);
	}
}

fxy_equation::fxy_equation(int graph_id, bool a) {
	r.gen();
	g_id = graph_id;
	active = a;
	exp = " ";
	should_remove = false;
}

bool fxy_equation::update(state* s, SDL_Event* ev) {
	if (active) {
		if (ev->type == SDL_TEXTINPUT) {
			if (exp == " ") exp.clear();
			exp.append(ev->text.text);
			return true;
		}
		if (ev->type == SDL_KEYDOWN) {
			int g_ind = getIndex(s, g_id);
			switch (ev->key.keysym.sym) {
			case SDLK_BACKSPACE:
				if (exp != " ") exp.pop_back();
				else {
					delete s->graphs[g_ind];
					s->graphs.erase(s->graphs.begin() + g_ind);
					should_remove = true;
					active = false;
					s->ev.callbacks.erase(remove_if(s->ev.callbacks.begin(), s->ev.callbacks.end(), [this](callback& c) -> bool { return c.id == g_id; }), s->ev.callbacks.end());
					s->ui->remove_dead_widgets();
					s->ev.current = in_ui;
					SDL_ShowCursor(1);
				}
				if (!exp.size()) exp = " ";
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

int fxy_equation::render(state* s, int w, int h, int ui_w, int x, int y, shader& program) {
	current_y = y;
	break_str(s, ui_w - x - 3);
	for (string l : lines) {
		SDL_Surface* text = TTF_RenderText_Shaded(s->font, l.c_str(), { 0, 0, 0 }, { 255, 255, 255 });
		r.tex.load(text);
		r.set((float)x, (float)y, (float)text->w, (float)text->h);
		SDL_FreeSurface(text);
		r.render(w, h, program);
		y += text->h;
	}
	current_yh = y;
	return y;
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
}
