
#pragma once

#include <SDL_ttf.h>
#include <SDL_opengl.h>
#include <string>

struct fxy_equation : public widget {
	fxy_equation(string str, bool a = false) {
		exp = str;
		active = a;
	}
	int render(int y_pos, int w, int total_w, int total_h, int xoffset, GLuint program) {
		current_y = y_pos;
		break_str(w - xoffset);
		for (string l : lines) {
			SDL_Surface* temp = TTF_RenderText_Shaded(font, l.c_str(), { 0, 0, 0 }, { 255, 255, 255 });
			SDL_Surface* text = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_RGB888, 0);
			pts[0] = { -1.0f + (float)xoffset / total_w                                  , 1.0f - 2.0f * (float)y_pos / total_h            , 0.0f, 1.0f };
			pts[1] = { -1.0f + (float)xoffset / total_w                                  , 1.0f - 2.0f * ((float)y_pos + text->h) / total_h, 0.0f, 0.0f };
			pts[2] = { -1.0f + (float)xoffset / total_w + 2.0f * (float)text->w / total_w, 1.0f - 2.0f * (float)y_pos / total_h            , 1.0f, 1.0f };

			pts[3] = { -1.0f + (float)xoffset / total_w                                  , 1.0f - 2.0f * ((float)y_pos + text->h) / total_h, 0.0f, 0.0f };
			pts[4] = { -1.0f + (float)xoffset / total_w + 2.0f * (float)text->w / total_w, 1.0f - 2.0f *  (float)y_pos / total_h           , 1.0f, 1.0f };
			pts[5] = { -1.0f + (float)xoffset / total_w + 2.0f * (float)text->w / total_w, 1.0f - 2.0f * ((float)y_pos + text->h) / total_h, 1.0f, 0.0f };
			y_pos += text->h;
			send();
			glBindVertexArray(VAO);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, text->w, text->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, text->pixels);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindVertexArray(0);
			gl_render(program);
			SDL_FreeSurface(temp);
			SDL_FreeSurface(text);
		}
		current_yh = y_pos;
		return y_pos;
	}
	bool process(SDL_Event ev, int w, state* s) {
		switch (ev.type) {
		case SDL_QUIT:
			return false;
		case SDL_MOUSEBUTTONDOWN:
			if (ev.button.x < w && ev.button.y > current_y && ev.button.y < current_yh) {
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
				else if (ev.key.keysym.sym == SDLK_RETURN ||
						 ev.key.keysym.sym == SDLK_RETURN2 ||
						 ev.key.keysym.sym == SDLK_KP_ENTER) {
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
};
