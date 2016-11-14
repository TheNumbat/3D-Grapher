
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "main.h"

using namespace std;

// TODO:
	// UI
		// Make GUI system
		// Axis scales
		// Text rendering
		// Text input
		// Actual input system
	// Rendering
		// Transparency, blending, maybe sorting
		// Lighting
		// Multiple graphs
		// Transparent UI
		// Antialiasing
	// Math Features
		// Parametric Eqs
		// Highlight curve along a set x/y
		// Partials
		// Level Curves
		// Vector feilds
		// 2D and 4D graphs
		// E Regions
		// Tangent Planes
		// Polar & Spherical Graphs
		// More functions (sec, csc, cot, max, min)
		// Negatives & remove asterisks
		// Maxs & mins (abs and rel)
	// Code
		// Speed up evaluation - NVM release build is way fast enough
		// Clean up postfix alg
	// Notes
		// To encode binary file as data: xxd -i infile.bin outfile.h

#define normw(x) (((float)x)/(s->w * 0.25f))
#define normh(x) (((float)x)/(s->h))

void loop(state* s);
void setup(state* s, int w, int h);
void kill(state* s);
int addMultiLineText(state* s, string str, float x, float y, float woffset, float hoffset);
void resetUI(state* s);
void regengraph(state* s);

int main(int argc, char** args) {

	state st;

	st.g.xmin = -25;
	st.g.xmax = 25;
	st.g.ymin = -25;
	st.g.ymax = 25;
	st.g.xrez = 250;
	st.g.yrez = 250;
	st.g.eq_str = "sin(x)*sin(y)*(sin(x)*sin(y))^(3/2)";

	setup(&st, 1280, 720);

	string exp;
	stringstream ss;

	ss << st.g.eq_str;
	in(ss, st.g.eq);
	printeq(cout, st.g.eq);

	Uint64 start = SDL_GetPerformanceCounter();
	gengraph(&st);
	Uint64 end = SDL_GetPerformanceCounter();
	cout << "time: " << (float)(end - start) / SDL_GetPerformanceFrequency() << endl;

	loop(&st);

	kill(&st);

	return 0;
}

void kill(state* s) {
	TTF_CloseFont(s->font);
	glDeleteBuffers(1, &s->axisVBO);
	glDeleteBuffers(1, &s->graphVBO);
	glDeleteBuffers(1, &s->EBO);
	glDeleteVertexArrays(1, &s->axisVAO);
	glDeleteVertexArrays(1, &s->graphVAO);
	SDL_GL_DeleteContext(s->context);
	SDL_DestroyWindow(s->window);
	SDL_Quit();
}

void loop(state* s) {

	int mx = (s->w - 250) / 2, my = s->h / 2;
	const unsigned char* keys = SDL_GetKeyboardState(NULL);

	glBindVertexArray(s->graphVAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, s->graphVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->EBO);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * s->g.indicies.size(), s->g.indicies.size() ? &s->g.indicies[0] : NULL, GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * s->g.verticies.size(), s->g.verticies.size() ? &s->g.verticies[0] : NULL, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
	}

	glBindVertexArray(s->axisVAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, s->axisVBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
	}

	while (s->running) {
		Uint64 start = SDL_GetPerformanceCounter();
		
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4 model, view, proj;
		view = getView(s->c);
		proj = perspective(radians(s->c.fov), (GLfloat)s->w / (GLfloat)s->h, 0.1f, 1000.0f);

		glBindVertexArray(s->graphVAO);
		{
			glUseProgram(s->graphShader);

			glUniformMatrix4fv(glGetUniformLocation(s->graphShader, "model"), 1, GL_FALSE, value_ptr(model));
			glUniformMatrix4fv(glGetUniformLocation(s->graphShader, "view"), 1, GL_FALSE, value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(s->graphShader, "proj"), 1, GL_FALSE, value_ptr(proj));

			glEnable(GL_DEPTH_TEST);

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glPolygonOffset(1.0f, 0.0f);
			
			glUniform4f(glGetUniformLocation(s->graphShader, "vcolor"), 0.8f, 0.8f, 0.8f, 1.0f);
			glDrawElements(GL_TRIANGLES, s->g.indicies.size(), GL_UNSIGNED_INT, (void*)0);

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glPolygonOffset(0.0f, 0.0f);
			
			glUniform4f(glGetUniformLocation(s->graphShader, "vcolor"), 0.2f, 0.2f, 0.2f, 1.0f);
			glDrawElements(GL_TRIANGLES, s->g.indicies.size(), GL_UNSIGNED_INT, (void*)0);
		}

		glBindVertexArray(s->axisVAO);
		{
			glUseProgram(s->axisShader);

			glUniformMatrix4fv(glGetUniformLocation(s->axisShader, "model"), 1, GL_FALSE, value_ptr(model));
			glUniformMatrix4fv(glGetUniformLocation(s->axisShader, "view"), 1, GL_FALSE, value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(s->axisShader, "proj"), 1, GL_FALSE, value_ptr(proj));

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDisable(GL_DEPTH_TEST);

			glDrawArrays(GL_LINES, 0, 6);
		}

		glViewport(0, round(0.25f * s->w) - 3, 3, s->h);
		glScissor(round(0.25f * s->w) - 3, 0, 3, s->h);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glScissor(0, 0, s->w, s->h);
		glViewport(0, 0, round(0.25f * s->w) - 3, s->h);
		s->ui.render(s->w, s->h);
		glViewport(round(0.25f * s->w), 0, round(0.75f * s->w), s->h);

		SDL_Event ev;
		while (SDL_PollEvent(&ev) != 0) {
			switch (ev.type) {
			case SDL_QUIT: {
				s->running = false;
				break;
			}
			case SDL_WINDOWEVENT: {
				if (ev.window.event == SDL_WINDOWEVENT_RESIZED) {
					s->w = ev.window.data1;
					s->h = ev.window.data2;
					resetUI(s);
					glViewport(0, 0, s->w, s->h);
				}
				break;
			}
			case SDL_MOUSEMOTION: {
				float sens = 0.1f;
				float dx = (ev.motion.x - mx) * sens;
				float dy = (ev.motion.y - my) * sens;
				mx = ev.motion.x;
				my = ev.motion.y;
				if (s->instate == in_cam) {					
					s->c.yaw += dx;
					s->c.pitch -= dy;
					if (s->c.yaw > 360.0f) s->c.yaw = 0.0f;
					else if (s->c.yaw < 0.0f) s->c.yaw = 360.0f;
					if (s->c.pitch > 89.9f) s->c.pitch = 89.9f;
					else if (s->c.pitch < -89.9f) s->c.pitch = -89.9f;
					updoot(s->c);
				}
				break;
			}
			case SDL_MOUSEWHEEL: {
				s->c.fov -= ev.wheel.y;
				if (s->c.fov > 179.0f) s->c.fov = 179.0f;
				else if (s->c.fov < 1.0f) s->c.fov = 1.0f;
				break;
			}
			case SDL_MOUSEBUTTONDOWN: {
				if (s->instate == in_idle && ev.button.x > round(0.25f * s->w)) {
					s->instate = in_cam;
					SDL_CaptureMouse(SDL_TRUE);
					SDL_SetRelativeMouseMode(SDL_TRUE);
				}
				int lastpx;
				for (graphelement& g : s->ui.gelements) {
					if (s->instate == in_idle && ev.button.x < round(0.25f * s->w) && ev.button.y > (g.pxoffset - 5) / 2 && ev.button.y < (g.pxoffset_bot + 5) / 2) {
						s->ui.selected = &g;
						s->instate = in_text;
						SDL_StartTextInput();
					}
					lastpx = g.pxoffset_bot;
				}
				if (s->instate == in_idle && ev.button.x < round(0.25f * s->w) && ev.button.y >(lastpx - 5) / 2 && ev.button.y < s->h) {
					addMultiLineText(s, " ", -1.0f, 1.0f, normw(10), normh(25 + lastpx));
					s->ui.selected = &s->ui.gelements.back();
					s->instate = in_text;
				}
				break;
			}
			case SDL_MOUSEBUTTONUP: {
				if (s->instate == in_cam) {
					s->instate = in_idle;
					SDL_CaptureMouse(SDL_FALSE);
					SDL_SetRelativeMouseMode(SDL_FALSE);
					SDL_WarpMouseInWindow(s->window, round(s->w * 0.625f), s->h / 2);
				}
				break;
			}
			case SDL_KEYDOWN: {
				switch (ev.key.keysym.sym) {
				case SDLK_ESCAPE:
					if (s->instate == in_text)
						s->instate = in_idle;
					break;
				case SDLK_BACKSPACE:
					if (s->instate == in_text) {
						if (s->ui.selected->str != " ") {
							s->ui.selected->str.pop_back();
							if (!s->ui.selected->str.size())
								s->ui.selected->str = " ";
							resetUI(s);
						}
						else {
							s->ui.gelements.erase(remove(s->ui.gelements.begin(), s->ui.gelements.end(), *s->ui.selected), s->ui.gelements.end());
							s->instate = in_idle;
							resetUI(s);
						}
					}
					break;
				case SDLK_RETURN:
					if (s->instate = in_text) {
						s->g.eq_str = s->ui.selected->str;
						regengraph(s);
						s->instate = in_idle;
					}
					break;
				}
				break;
			}
			case SDL_TEXTINPUT: {
				if (s->instate == in_text) {
					s->ui.selected->str += ev.text.text;
					resetUI(s);
				}
				break;
			}
			}
		}
		float dT = (SDL_GetTicks() - s->c.lastUpdate) / 1000.0f;
		s->c.lastUpdate = SDL_GetTicks();
		if (s->instate == in_cam) {
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

		SDL_GL_SwapWindow(s->window);

		Uint64 end = SDL_GetPerformanceCounter();
		//cout << "frame: " << 1000.0f * (end - start) / (float)SDL_GetPerformanceFrequency() << "ms" << endl;
	}
}

void regengraph(state* s) {
	s->g.indicies.clear();
	s->g.verticies.clear();
	s->g.eq.clear();

	stringstream ss;

	ss << s->g.eq_str;
	in(ss, s->g.eq);
	printeq(cout, s->g.eq);

	Uint64 start = SDL_GetPerformanceCounter();
	gengraph(s);
	Uint64 end = SDL_GetPerformanceCounter();
	cout << "time: " << (float)(end - start) / SDL_GetPerformanceFrequency() << endl;

	glBindVertexArray(s->graphVAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, s->graphVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->EBO);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * s->g.indicies.size(), s->g.indicies.size() ? &s->g.indicies[0] : NULL, GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * s->g.verticies.size(), s->g.verticies.size() ? &s->g.verticies[0] : NULL, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
	}

	glBindVertexArray(s->axisVAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, s->axisVBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
	}
}

void setup(state* s, int w, int h) {

	s->w = w;
	s->h = h;

	SDL_Init(SDL_INIT_EVERYTHING);

	s->window = SDL_CreateWindow("3D Grapher", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	assert(s->window);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	s->context = SDL_GL_CreateContext(s->window);
	assert(s->context);

	setupFuns();

	glEnable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, w - 250, h);

	SDL_GL_SetSwapInterval(-1);

	GLuint vert, frag, cvert, cfrag;
	vert = glCreateShader(GL_VERTEX_SHADER);
	cvert = glCreateShader(GL_VERTEX_SHADER);
	frag = glCreateShader(GL_FRAGMENT_SHADER);
	cfrag = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vert, 1, &vertex, NULL);
	glShaderSource(cvert, 1, &colorvertex, NULL);
	glShaderSource(frag, 1, &fragment, NULL);
	glShaderSource(cfrag, 1, &colorfragment, NULL);

	glCompileShader(vert);
	glCompileShader(cvert);
	glCompileShader(frag);
	glCompileShader(cfrag);

	s->graphShader = glCreateProgram();
	glAttachShader(s->graphShader, vert);
	glAttachShader(s->graphShader, frag);
	glLinkProgram(s->graphShader);

	s->axisShader = glCreateProgram();
	glAttachShader(s->axisShader, cvert);
	glAttachShader(s->axisShader, cfrag);
	glLinkProgram(s->axisShader);

	glDeleteShader(vert);
	glDeleteShader(cvert);
	glDeleteShader(frag);
	glDeleteShader(cfrag);

	glGenVertexArrays(1, &s->axisVAO);
	glGenVertexArrays(1, &s->graphVAO);
	glGenBuffers(1, &s->axisVBO);
	glGenBuffers(1, &s->graphVBO);
	glGenBuffers(1, &s->EBO);

	TTF_Init();
	s->font = TTF_OpenFontRW(SDL_RWFromConstMem((const void*)DroidSans_ttf, DroidSans_ttf_len), 1, 48);

	s->c = defaultCam();
	s->running = true;
	s->instate = in_idle;

	s->ui.init();
	resetUI(s);

	int offset = 0;
	offset = addMultiLineText(s, "EQUATIONS", -1.0f, 1.0f, normw(10), normh(25 + offset));
	offset = addMultiLineText(s, s->g.eq_str, -1.0f, 1.0f, normw(10), normh(25 + offset));
}

int addMultiLineText(state* s, string str, float x, float y, float woffset, float hoffset) {
	graphelement ge;
	ge.str = str;
	ge.pxoffset = hoffset * s->h;
	int tw, th;
	TTF_SizeText(s->font, str.c_str(), &tw, &th);
	int stroffset = 0;
	for (float i = normw(tw); i > 0 && stroffset != str.size(); i -= 1.0f)
	{
		int end = stroffset + 1;
		if (str.size() > 1) {
			tw = 0;
			do {
				if (end == str.size()) break;
				TTF_SizeText(s->font, str.substr(stroffset, end - stroffset).c_str(), &tw, &th);
				end++;
			} while (tw < (1.0f - x) * round(0.23f * s->w));
		}
		float normW = normw(tw);
		float normH = normh(th);

		UItext* text = new UItext(s->font, str.substr(stroffset, end - stroffset));
		stroffset = end;

		text->points[0] = { x + woffset		   , y - hoffset		, 0.0f, 1.0f };
		text->points[1] = { x + normW + woffset, y - hoffset		, 1.0f, 1.0f };
		text->points[2] = { x + woffset		   , y - normH - hoffset, 0.0f, 0.0f };

		text->points[3] = { x + normW + woffset, y - hoffset		, 1.0f, 1.0f };
		text->points[4] = { x + woffset		   , y - normH - hoffset, 0.0f, 0.0f };
		text->points[5] = { x + normW + woffset, y - normH - hoffset, 1.0f, 0.0f };

		hoffset += normH;

		s->ui.elements.push_back(text);
		ge.UIelements.push_back(s->ui.elements.size() - 1);
	}
	ge.pxoffset_bot = hoffset * s->h;
	s->ui.gelements.push_back(ge);
	return hoffset * s->h;
}

void resetUI(state* s) {
	vector<string> gstrs = s->ui.reset();
	int offset = 0;
	for(string& str : gstrs)
		offset = addMultiLineText(s, str, -1.0f, 1.0f, normw(10), normh(25 + offset));
}