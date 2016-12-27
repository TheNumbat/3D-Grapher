
#include "state.h"
#include "data/font.data"

int next_graph_id = 0;
int mx = screen_w / 2, my = screen_h / 2;

state::state() {
	w = screen_w;
	h = screen_h;

	assert(SDL_Init(SDL_INIT_EVERYTHING) == 0);

	window = SDL_CreateWindow("3D Grapher", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screen_w, screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	assert(window);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	context = SDL_GL_CreateContext(window);
	assert(context);

	SDL_GL_SetSwapInterval(1);
	setupFuns();

	glEnable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, w, h);

	glGenVertexArrays(1, &axisVAO);
	glGenBuffers(1, &axisVBO);

	graph_s.load(graph_vertex, graph_fragment);
	axis_s.load(axis_vertex, axis_fragment);
	UI_s.load(ui_vertex, ui_fragment);
	rect_s.load(rect_vertex, rect_fragment);

	TTF_Init();
	font = TTF_OpenFontRW(SDL_RWFromConstMem((const void*)DroidSans_ttf, DroidSans_ttf_len), 1, 24);
	
	ui = new UI();

	add_default_callbacks(this);
	sendAxes(this);

	c.default();
	running = true;
}

state::~state() {
	delete ui;
	TTF_CloseFont(font);
	glDeleteBuffers(1, &axisVBO);
	glDeleteVertexArrays(1, &axisVAO);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void state::run() {
	const unsigned char* keys = SDL_GetKeyboardState(NULL);

	while (running) {
		Uint64 start = SDL_GetPerformanceCounter();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4 model, view, proj, modelviewproj;
		view = c.getView();
		proj = perspective(radians(c.fov), (GLfloat)w / (GLfloat)h, 0.1f, 1000.0f);
		modelviewproj = proj * view * model;

		for (graph* g : graphs) {
			if (g->indicies.size()) {
				glBindVertexArray(g->VAO);
				{
					graph_s.use();

					glBindBuffer(GL_ARRAY_BUFFER, g->VBO);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g->EBO);

					glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
					glEnableVertexAttribArray(0);

					glUniformMatrix4fv(graph_s.getUniform("modelviewproj"), 1, GL_FALSE, value_ptr(modelviewproj));

					glEnable(GL_DEPTH_TEST);
					glDisable(GL_BLEND);

					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glPolygonOffset(1.0f, 0.0f);

					glUniform4f(graph_s.getUniform("vcolor"), 0.8f, 0.8f, 0.8f, 1.0f);
					glDrawElements(GL_TRIANGLES, (int)g->indicies.size(), GL_UNSIGNED_INT, (void*)0);

					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glPolygonOffset(0.0f, 0.0f);

					glUniform4f(graph_s.getUniform("vcolor"), 0.2f, 0.2f, 0.2f, 1.0f);
					glDrawElements(GL_TRIANGLES, (int)g->indicies.size(), GL_UNSIGNED_INT, (void*)0);

					glDisableVertexAttribArray(0);
				}
				glBindVertexArray(0);
			}
		}

		glBindVertexArray(axisVAO);
		{
			axis_s.use();

			glBindBuffer(GL_ARRAY_BUFFER, axisVBO);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
			glEnableVertexAttribArray(1);

			glUniformMatrix4fv(axis_s.getUniform("modelviewproj"), 1, GL_FALSE, value_ptr(modelviewproj));

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
			ui->render(this, w, h, UI_s, rect_s);
		}

		ev.run(this);

		SDL_GL_SwapWindow(window);

		Uint64 end = SDL_GetPerformanceCounter();
		//cout << "frame: " << 1000.0f * (end - start) / (float)SDL_GetPerformanceFrequency() << "ms" << endl;
	}
}