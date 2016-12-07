
#pragma once

#include <SDL_ttf.h>
#include <SDL_opengl.h>
#include <string>

struct fxy_equation : public widget {
	fxy_equation() {
		callback = [this](state* s) -> void {s->g.eq_str = exp; regengraph(s);};
	}
	int render(int y_pos, int w, int h, int xoffset, GLuint program) {
		break_str(w - xoffset);
		for (string l : lines) {
			SDL_Surface* temp = TTF_RenderText_Shaded(font, l.c_str(), { 0, 0, 0 }, { 255, 255, 255 });
			SDL_Surface* text = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_RGB888, 0);
			pts[0] = { -1.0f + (float)xoffset / w                            , 1.0f - 2.0f * (float)y_pos / h            , 0.0f, 1.0f };
			pts[1] = { -1.0f + (float)xoffset / w                            , 1.0f - 2.0f * ((float)y_pos + text->h) / h, 0.0f, 0.0f };
			pts[2] = { -1.0f + (float)xoffset / w + 2.0f * (float)text->w / w, 1.0f - 2.0f * (float)y_pos / h            , 1.0f, 1.0f };

			pts[3] = { -1.0f + (float)xoffset / w                            , 1.0f - 2.0f * ((float)y_pos + text->h) / h, 0.0f, 0.0f };
			pts[4] = { -1.0f + (float)xoffset / w + 2.0f * (float)text->w / w, 1.0f - 2.0f *  (float)y_pos / h           , 1.0f, 1.0f };
			pts[5] = { -1.0f + (float)xoffset / w + 2.0f * (float)text->w / w, 1.0f - 2.0f * ((float)y_pos + text->h) / h, 1.0f, 0.0f };
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
		return y_pos;
	}
	void process(SDL_Event ev) {

	}
	void break_str(int w) {
		lines.clear();
		int e_pos = 0, tw;
		TTF_SizeText(font, exp.c_str(), &tw, NULL);
		do {
			int end = e_pos, newtw;
			do {
				end++;
				TTF_SizeText(font, exp.substr(e_pos, end - e_pos).c_str(), &newtw, NULL);
				if (end == exp.size()) end++;
			} while (end < exp.size() && newtw < w);
			end--;
			lines.push_back(exp.substr(e_pos, end - e_pos));
			e_pos = end;
			tw -= newtw;
		} while (tw > 0);
	}
	string exp;
	vector<string> lines;
};
