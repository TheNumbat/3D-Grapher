
#include <iostream>
#include <fstream>
#include <string>

const float UI_SCREEN_RATIO = 0.25f;

#include "font.data"
#include "glfuns.h"
#include "types.h"
#include "shaders.h"
#include "graph.h"
#include "cam.h"
#include "ui.h"

using namespace std;

// TODO:
	// UI
		// Better desmos-style UI
		// Axis scales
		// Actual input system
	// Rendering
		// Transparency, blending, maybe sorting
		// Lighting
		// Multiple graphs
		// Semitransparent UI
		// Antialiasing
		// Axis normalization
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

void loop(state* s);
void setup(state* s, int w, int h);
void kill(state* s);

int main(int argc, char** args) {

	state st;

	st.g.xmin = -10;
	st.g.xmax = 10;
	st.g.ymin = -10;
	st.g.ymax = 10;
	st.g.xrez = 200;
	st.g.yrez = 200;
	st.g.eq_str = "sin(x)*sin(y)*sin(x)*sin(y)";

	setup(&st, 1280, 720);
	regengraph(&st);
	loop(&st);
	kill(&st);

	return 0;
}

void kill(state* s) {
	TTF_CloseFont(font);
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

	int mx = s->w / 2, my = s->h / 2;
	const unsigned char* keys = SDL_GetKeyboardState(NULL);

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
			glDrawElements(GL_TRIANGLES, (int)s->g.indicies.size(), GL_UNSIGNED_INT, (void*)0);

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glPolygonOffset(0.0f, 0.0f);
			
			glUniform4f(glGetUniformLocation(s->graphShader, "vcolor"), 0.2f, 0.2f, 0.2f, 1.0f);
			glDrawElements(GL_TRIANGLES, (int)s->g.indicies.size(), GL_UNSIGNED_INT, (void*)0);
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

		glScissor((int)round(s->w * UI_SCREEN_RATIO), 0, 3, s->h);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glScissor(0, 0, (int)round(s->w * UI_SCREEN_RATIO), s->h);
		glViewport(0, 0, (int)round(s->w * UI_SCREEN_RATIO), s->h);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		s->ui.render((int)round(s->w * UI_SCREEN_RATIO), s->h, s->uiShader);
		glScissor((int)round(s->w * UI_SCREEN_RATIO) + 3, 0, (int)round(s->w * (1 - UI_SCREEN_RATIO)), s->h);
		glViewport((int)round(s->w * UI_SCREEN_RATIO) + 3, 0, (int)round(s->w * (1 - UI_SCREEN_RATIO)), s->h);

		SDL_Event ev;
		while (SDL_PollEvent(&ev) != 0) {
			switch (ev.type) {
			case SDL_QUIT: {
				s->running = false;
				break;
			}
			case SDL_WINDOWEVENT: {
				if (ev.window.event == SDL_WINDOWEVENT_RESIZED ||
					ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
					s->w = ev.window.data1;
					s->h = ev.window.data2;
					glViewport((int)round(s->w * UI_SCREEN_RATIO) + 3, 0, (int)round(s->w * (1 - UI_SCREEN_RATIO)), s->h);
					glScissor((int)round(s->w * UI_SCREEN_RATIO) + 3, 0, (int)round(s->w * (1 - UI_SCREEN_RATIO)), s->h);
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
					if (s->c.pitch > 89.0f) s->c.pitch = 89.0f;
					else if (s->c.pitch < -89.0f) s->c.pitch = -89.0f;
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
				if (s->instate == in_idle) {
					s->instate = in_cam;
					SDL_CaptureMouse(SDL_TRUE);
					SDL_SetRelativeMouseMode(SDL_TRUE);
				}
				break;
			}
			case SDL_MOUSEBUTTONUP: {
				if (s->instate == in_cam) {
					s->instate = in_idle;
					SDL_CaptureMouse(SDL_FALSE);
					SDL_SetRelativeMouseMode(SDL_FALSE);
					SDL_WarpMouseInWindow(s->window, (int)round(s->w * ((1 - UI_SCREEN_RATIO) / 2 + UI_SCREEN_RATIO)), s->h / 2);
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
	glViewport((int)round(s->w * UI_SCREEN_RATIO) + 3, 0, (int)round(s->w * (1 - UI_SCREEN_RATIO)), s->h);

	SDL_GL_SetSwapInterval(-1);

	GLuint vert, frag, cvert, cfrag, tvert, tfrag;
	vert = glCreateShader(GL_VERTEX_SHADER);
	cvert = glCreateShader(GL_VERTEX_SHADER);
	tvert = glCreateShader(GL_VERTEX_SHADER);
	frag = glCreateShader(GL_FRAGMENT_SHADER);
	cfrag = glCreateShader(GL_FRAGMENT_SHADER);
	tfrag = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vert, 1, &vertex, NULL);
	glShaderSource(cvert, 1, &colorvertex, NULL);
	glShaderSource(frag, 1, &fragment, NULL);
	glShaderSource(cfrag, 1, &colorfragment, NULL);
	glShaderSource(tvert, 1, &vtextured2D, NULL);
	glShaderSource(tfrag, 1, &ftextured2D, NULL);

	glCompileShader(vert);
	glCompileShader(cvert);
	glCompileShader(tvert);
	glCompileShader(frag);
	glCompileShader(cfrag);
	glCompileShader(tfrag);

	s->graphShader = glCreateProgram();
	glAttachShader(s->graphShader, vert);
	glAttachShader(s->graphShader, frag);
	glLinkProgram(s->graphShader);

	s->axisShader = glCreateProgram();
	glAttachShader(s->axisShader, cvert);
	glAttachShader(s->axisShader, cfrag);
	glLinkProgram(s->axisShader);

	s->uiShader = glCreateProgram();
	glAttachShader(s->uiShader, tvert);
	glAttachShader(s->uiShader, tfrag);
	glLinkProgram(s->uiShader);

	glDeleteShader(vert);
	glDeleteShader(cvert);
	glDeleteShader(tvert);
	glDeleteShader(frag);
	glDeleteShader(cfrag);
	glDeleteShader(tfrag);

	glGenVertexArrays(1, &s->axisVAO);
	glGenVertexArrays(1, &s->graphVAO);
	glGenBuffers(1, &s->axisVBO);
	glGenBuffers(1, &s->graphVBO);
	glGenBuffers(1, &s->EBO);

	TTF_Init();
	font = TTF_OpenFontRW(SDL_RWFromConstMem((const void*)DroidSans_ttf, DroidSans_ttf_len), 1, 24);

	{
		fxy_equation* eqw = new fxy_equation();
		eqw->exp = s->g.eq_str;
		s->ui.widgets.push_back(eqw);
	}
	{
		fxy_equation* eqw = new fxy_equation();
		eqw->exp = s->g.eq_str;
		s->ui.widgets.push_back(eqw);
	}
	{
		fxy_equation* eqw = new fxy_equation();
		eqw->exp = s->g.eq_str;
		s->ui.widgets.push_back(eqw);
	}

	s->c = defaultCam();
	s->running = true;
	s->instate = in_idle;
}