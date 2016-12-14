
#pragma once

#include <SDL_ttf.h>
#include <SDL_opengl.h>
#include <string>

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
					s->g.eq_str = exp; 
					regengraph(s);
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
