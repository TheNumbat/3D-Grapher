
#pragma once

#include <SDL_ttf.h>
#include <SDL_opengl.h>
#include <string>

TTF_Font* font;

#include "gl.h"
#include "graph.h"
#include "tex_in.data"
#include "tex_out.data"
#include "tex_gear.data"

struct UI {
	UI() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		active = false;
		in.tex.load(SDL_LoadBMP_RW(SDL_RWFromConstMem(in_bmp, in_bmp_len), 1));
		out.tex.load(SDL_LoadBMP_RW(SDL_RWFromConstMem(out_bmp, out_bmp_len), 1));
		gear.tex.load(SDL_LoadBMP_RW(SDL_RWFromConstMem(gear_bmp, gear_bmp_len), 1));
	}
	~UI() {
		for (widget* w : widgets)
			delete w;
		widgets.clear();
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}
	void remove_dead_widgets() {
		for (unsigned int i = 0; i < widgets.size(); i++) {
			if (widgets[i]->should_remove) {
				delete widgets[i];
				widgets.erase(widgets.begin() + i);
			}
		}
	}
	void drawRect(shader& shader, int x, int y, int w, int h, float r, float g, float b, float a, float screenw, float screenh) {
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
	void render(int w, int h, shader& ui_s, shader& rect_s) {
		float fw = (float)w, fh = (float)h;
		int xoff, ui_w = (int)round(w * UI_SCREEN_RATIO);
		if (active) {
			xoff = 0;
			in.set(ui_w + 5.0f, 0, 32, 32);
			in.render(w, h, ui_s);
			gear.set(ui_w + 5.0f, 35, 32, 32);
			gear.render(w, h, ui_s);
		}
		else {
			xoff = -ui_w + 5;
			out.set(11, 0, 32, 32);
			out.render(w, h, ui_s);
		}
		drawRect(rect_s, xoff, 0, ui_w, h, 1.0f, 1.0f, 1.0f, 1.0f, fw, fh); // white background
		drawRect(rect_s, xoff + ui_w, 0, 3, h, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // right black strip
		int cur_y = 0;
		unsigned int windex = 0;
		while (cur_y < h && windex < widgets.size()) {
			drawRect(rect_s, xoff, cur_y, ui_w, 3, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // top/bottom black strip
			cur_y += 3;
			cur_y = widgets[windex++]->render(w, h, ui_w, 5 + xoff, cur_y, ui_s); // widget
			cur_y += 3;
		}
		drawRect(rect_s, xoff, cur_y, ui_w, 3, 0.0f, 0.0f, 0.0f, 1.0f, fw, fh); // bottom black strip
	}
	vector<widget*> widgets;
	GLuint VAO, VBO;
	textured_rect in, out, gear;
	bool active;
};

struct fxy_equation : public widget {
	fxy_equation(string str, bool a = false) {
		exp = str;
		active = a;
		should_remove = false;
	}
	int render(int w, int h, int ui_w, int x, int y, shader& program) {
		current_y = y;
		break_str(ui_w - x);
		for (string l : lines) {
			SDL_Surface* text = TTF_RenderText_Shaded(font, l.c_str(), { 0, 0, 0 }, { 255, 255, 255 });
			r.tex.load(text);
			r.set((float)x, (float)y, (float)text->w, (float)text->h);
			SDL_FreeSurface(text);
			r.render(w, h, program);
			y += text->h;
		}
		current_yh = y;
		return y;
	}
	bool process(SDL_Event ev, int w, state* s) {
		switch (ev.type) {
		case SDL_QUIT:
			return false;
		case SDL_MOUSEBUTTONDOWN:
			if (ev.button.x < w && ev.button.y >= current_y && ev.button.y <= current_yh) {
				active = true;
				SDL_StartTextInput();
				return true;
			}
			break;
		case SDL_TEXTINPUT:
			if (active) {
				if (exp == " ") exp.clear();
				exp.append(ev.text.text);
			}
			break;
		case SDL_KEYDOWN:
			if (active) {
				if (ev.key.keysym.sym == SDLK_ESCAPE) {
					active = false;
					SDL_StopTextInput();
				}
				else if (exp != " " && (
					     ev.key.keysym.sym == SDLK_RETURN ||
						 ev.key.keysym.sym == SDLK_RETURN2 ||
						 ev.key.keysym.sym == SDLK_KP_ENTER)) {
					active = false;
					s->graphs[0].eq_str = exp; 
					regengraph(s, 0);
				}
				else if (ev.key.keysym.sym == SDLK_BACKSPACE) {
					if (exp != " ") exp.pop_back();
					else should_remove = true;
					if (!exp.size()) exp = " ";
				}
			}
			break;
		}
		return active;
	}
	void break_str(int w) {
		lines.clear();
		int e_pos = 0, tw;
		TTF_SizeText(font, exp.c_str(), &tw, NULL);
		do {
			unsigned int end = e_pos;
			int newtw;
			do {
				end++;
				TTF_SizeText(font, exp.substr(e_pos, end - e_pos).c_str(), &newtw, NULL);
			} while (end < exp.size() && newtw < w);
			if (newtw > w) {
				end--;
				TTF_SizeText(font, exp.substr(e_pos, end - e_pos).c_str(), &newtw, NULL);
			}
			lines.push_back(exp.substr(e_pos, end - e_pos));
			e_pos = end;
			tw -= newtw;
		} while (tw > 0);
	}
	string exp;
	vector<string> lines;
	textured_rect r;
};
