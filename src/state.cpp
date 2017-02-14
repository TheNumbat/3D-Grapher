
#include "state.h"
#include "font.data"
#include <algorithm>

state::state() {
	w = screen_w;
	h = screen_h;
	mx = w / 2;
	my = h / 2;
	next_graph_id = 0;
	last_mx = last_my = 0;

	set.wireframe = false;
	set.lighting = true;
	set.axisnormalization = false;
	set.graphopacity = 1.0f;
	set.ambientLighting = 0.0f;
	set.fov = 60.0f;
	set.camtype = cam_3d_static;
	set.rdom = { -10, 10, -10, 10, -10, 10, 200, 200 };
	set.cdom = { 0, 1, 0, 2 * val_pi, 0, 10, 200, 200 };
	set.sdom = { 0, 2 * val_pi, 0, 2 * val_pi, 0, 10, 200, 200 };

	assert(SDL_Init(SDL_INIT_EVERYTHING) == 0);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	window = SDL_CreateWindow("3D Grapher", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screen_w, screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	assert(window);

	context = SDL_GL_CreateContext(window);
	assert(context);

	SDL_GL_SetSwapInterval(1);
	setupFuns();

	glEnable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, w, h);

	glGenVertexArrays(1, &axisVAO);
	glGenBuffers(1, &axisVBO);

	graph_s.load(graph_vertex, graph_fragment);
	axis_s.load(axis_vertex, axis_fragment);
	UI_s.load(ui_vertex, ui_fragment);
	rect_s.load(rect_vertex, rect_fragment);
	graph_s_light.load(graph_vertex_lighting, graph_fragment_lighting);

	TTF_Init();
	font = TTF_OpenFontRW(SDL_RWFromConstMem((const void*)DroidSans_ttf, DroidSans_ttf_len), 1, 24);
	
	add_default_callbacks(this);
	updateAxes(this);

	ui = new UI(this);

	c_3d.reset();
	c_3d_static.reset();
	running = true;
}

state::~state() {
	delete ui;
	for (graph* g : graphs)
		delete g;
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
		model = rotate(model, radians(-90.0f), vec3(1, 0, 0));
		if (set.camtype == cam_3d) {
			view = c_3d.getView();
			proj = perspective(radians(set.fov), (GLfloat)w / (GLfloat)h, 0.1f, 1000.0f);
		}
		else {
			view = c_3d_static.getView();
			proj = perspective(radians(set.fov), (GLfloat)w / (GLfloat)h, 0.1f, 1000.0f);
		}
		modelviewproj = proj * view * model;

		for (graph* g : graphs) {
			g->draw(this, model, view, proj);
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
			ui->render(this);
		}

		ev.run(this);

		SDL_GL_SwapWindow(window);

		Uint64 end = SDL_GetPerformanceCounter();
		//cout << "frame: " << 1000.0f * (end - start) / (float)SDL_GetPerformanceFrequency() << "ms" << endl;
	}
}