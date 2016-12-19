
#include <iostream>
#include <fstream>
#include <string>

const float UI_SCREEN_RATIO = 0.2f;
const int w = 1280, h = 720;
int next_graph_id = 0;
int mx = w / 2, my = h / 2;

#include "font.data"
#include "types.h"
#include "graph.h"
#include "cam.h"

using namespace std;

// TODO:
	// UI
		// Improve UI
		// Settings
		// Axis scales
		// Better Events System (registry?)
	// Rendering
		// Transparency, blending, maybe sorting
		// Lighting
		// Antialiasing
	// Math Features
		// Axis normalization
		// Highlight curve along a set x/y
		// Partials
		// Graph intersections
		// Level Curves
		// Vector feilds
		// 2D and 4D graphs
		// E Regions
		// Tangent Planes
		// Polar & Spherical Graphs
		// More functions (sec, csc, cot, max, min)
		// Negatives & remove asterisks
		// Maxs & mins (abs and rel)
		// Parametric Lines & Surfaces
		// Line/Surface/Flux integrals
	// Notes
		// To encode binary file as data array: xxd -i infile.bin outfile.h

void loop(state* s);
void setup(state* s);
void kill(state* s);

int main(int argc, char** args) {

	state st;

	setup(&st);
	loop(&st);
	kill(&st);

	return 0;
}

void kill(state* s) {
	delete s->ui;
	delete s->ev;
	delete s->c;
	TTF_CloseFont(font);
	glDeleteBuffers(1, &s->axisVBO);
	glDeleteVertexArrays(1, &s->axisVAO);
	SDL_GL_DeleteContext(s->context);
	SDL_DestroyWindow(s->window);
	SDL_Quit();
}

void loop(state* s) {

	const unsigned char* keys = SDL_GetKeyboardState(NULL);

	while (s->running) {
		Uint64 start = SDL_GetPerformanceCounter();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4 model, view, proj, modelviewproj;
		view = s->c->getView();
		proj = perspective(radians(s->c->fov), (GLfloat)s->w / (GLfloat)s->h, 0.1f, 1000.0f);
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

		s->ev->run(s);

		SDL_GL_SwapWindow(s->window);

		Uint64 end = SDL_GetPerformanceCounter();
		//cout << "frame: " << 1000.0f * (end - start) / (float)SDL_GetPerformanceFrequency() << "ms" << endl;
	}
}

void setup(state* s) {
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

	s->ev = new evts();
	s->ui = new UI();
	s->c = new cam();

	add_default_callbacks(s);
	sendAxes(s);

	s->c->default();
	s->running = true;
}