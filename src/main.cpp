﻿
#include <iostream>
#include <fstream>
#include <string>

const float UI_SCREEN_RATIO = 0.2f;
int next_graph_id = 0;

#include "font.data"
#include "types.h"
#include "graph.h"
#include "cam.h"

using namespace std;

// TODO:
	// UI
		// Better desmos-style UI
		// Axis scales
		// Legit UI system? Or just do janky stuff?
	// Rendering
		// Transparency, blending, maybe sorting
		// Lighting
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

	setup(&st, 1280, 720);
	loop(&st);
	kill(&st);

	return 0;
}

void kill(state* s) {
	delete s->ui;
	TTF_CloseFont(font);
	glDeleteBuffers(1, &s->axisVBO);
	glDeleteVertexArrays(1, &s->axisVAO);
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

		mat4 model, view, proj, modelviewproj;
		view = getView(s->c);
		proj = perspective(radians(s->c.fov), (GLfloat)s->w / (GLfloat)s->h, 0.1f, 1000.0f);
		modelviewproj = proj * view * model;

		for (graph* g : s->graphs) {
			if (g->indicies.size()) {
				glBindVertexArray(g->VAO);
				{
					s->graph_s.use();

					glBindBuffer(GL_ARRAY_BUFFER, g->VBO);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g->EBO);

					glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
					glEnableVertexAttribArray(0);

					glUniformMatrix4fv(s->graph_s.getUniform("modelviewproj"), 1, GL_FALSE, value_ptr(modelviewproj));

					glEnable(GL_DEPTH_TEST);
					glDisable(GL_BLEND);

					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glPolygonOffset(1.0f, 0.0f);

					glUniform4f(s->graph_s.getUniform("vcolor"), 0.8f, 0.8f, 0.8f, 1.0f);
					glDrawElements(GL_TRIANGLES, (int)g->indicies.size(), GL_UNSIGNED_INT, (void*)0);

					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glPolygonOffset(0.0f, 0.0f);

					glUniform4f(s->graph_s.getUniform("vcolor"), 0.2f, 0.2f, 0.2f, 1.0f);
					glDrawElements(GL_TRIANGLES, (int)g->indicies.size(), GL_UNSIGNED_INT, (void*)0);

					glDisableVertexAttribArray(0);
				}
				glBindVertexArray(0);
			}
		}
			
		glBindVertexArray(s->axisVAO);
		{
			s->axis_s.use();

			glBindBuffer(GL_ARRAY_BUFFER, s->axisVBO);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
			glEnableVertexAttribArray(1);

			glUniformMatrix4fv(s->axis_s.getUniform("modelviewproj"), 1, GL_FALSE, value_ptr(modelviewproj));

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);

			glDrawArrays(GL_LINES, 0, 6);

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
		}
		glBindVertexArray(0);

		{
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			s->ui->render(s->w, s->h, s->UI_s, s->rect_s);
		}

		SDL_Event ev;
		while (SDL_PollEvent(&ev) != 0) {
			bool intercepted = false;
			if (s->ui->active) {
				for (widget* w : s->ui->widgets) {
					intercepted = w->process(ev, (int)round(s->w * UI_SCREEN_RATIO), s);
					if (intercepted) break;
				}
			}
			s->ui->remove_dead_widgets();
			if (intercepted) continue;
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
					glViewport(0, 0, s->w, s->h);
					glScissor(0, 0, s->w, s->h);
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
					if (s->ui->active) {
						if (ev.button.x < (int)round(s->w * UI_SCREEN_RATIO)) {
							s->graphs.push_back(new graph(next_graph_id, "", -10, 10, -10, 10, 200, 200));
							s->graphs.back()->gen();
							s->ui->widgets.push_back(new fxy_equation(" ", next_graph_id, true));
							next_graph_id++;
						}
						else if (ev.button.x > (int)round(s->w * UI_SCREEN_RATIO) && ev.button.x <= (int)round(s->w * UI_SCREEN_RATIO) + 37 &&
								 ev.button.y <= 32) {
							s->ui->active = false;
						}
						else {
							s->instate = in_cam;
							SDL_CaptureMouse(SDL_TRUE);
							SDL_SetRelativeMouseMode(SDL_TRUE);
						}
					}
					else if (ev.button.x <= 43 && ev.button.y <= 32) {
						s->ui->active = true;
					}
					else {
						s->instate = in_cam;
						SDL_CaptureMouse(SDL_TRUE);
						SDL_SetRelativeMouseMode(SDL_TRUE);
					}
				}
				break;
			}
			case SDL_MOUSEBUTTONUP: {
				if (s->instate == in_cam) {
					s->instate = in_idle;
					SDL_CaptureMouse(SDL_FALSE);
					SDL_SetRelativeMouseMode(SDL_FALSE);
					SDL_WarpMouseInWindow(s->window, s->ui->active ? (int)round(s->w * ((1 - UI_SCREEN_RATIO) / 2 + UI_SCREEN_RATIO)) : s->w / 2, s->h / 2);
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

	assert(SDL_Init(SDL_INIT_EVERYTHING) == 0);

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

	SDL_GL_SetSwapInterval(1);
	setupFuns();

	glEnable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, s->w, s->h);

	glGenVertexArrays(1, &s->axisVAO);
	glGenBuffers(1, &s->axisVBO);

	s->graph_s.load(graph_vertex, graph_fragment);
	s->axis_s.load(axis_vertex, axis_fragment);
	s->UI_s.load(ui_vertex, ui_fragment);
	s->rect_s.load(rect_vertex, rect_fragment);

	TTF_Init();
	font = TTF_OpenFontRW(SDL_RWFromConstMem((const void*)DroidSans_ttf, DroidSans_ttf_len), 1, 24);

	s->ui = new UI();
	sendAxes(s);
	s->c = defaultCam();
	s->running = true;
	s->instate = in_idle;
}