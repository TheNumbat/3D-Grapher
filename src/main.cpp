﻿
#include "exp.h"
#include "main.h"

using namespace std;

// TODO:
	// UI
		// Axis scales
		// Text rendering
		// Text input
		// Actual input system
	// Rendering
		// Camera movement, look, pan, etc.
		// Transparency, blending, maybe sorting
		// Lighting
		// Multiple graphs
	// More features
		// Partials
		// Double integrals?
		// Probably a ton more stuff I can't think of right now

void loop(state* s);
void setup(state* s, int w, int h);
void kill(state* s);
void gengraph(state* s);

int main(int argc, char** args) {
	
	state st;
	setup(&st, 640, 480);

	welcome(cout);
	string exp;
	stringstream ss;
	cout << "xmin: ";
	cin >> st.g.xmin;
	cout << "xmax: ";
	cin >> st.g.xmax;
	cout << "ymin: ";
	cin >> st.g.ymin;
	cout << "ymax: ";
	cin >> st.g.ymax;
	cout << "xrez: ";
	cin >> st.g.xrez;
	cout << "yrez: ";
	cin >> st.g.yrez;
	cout << "exp: ";

	cin.ignore();
	getline(cin, exp);

	ss << exp;
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

float clamp(float one, float two) {
	if (one > 0)
		return one - fmod(one, two);
	else if (one < 0)
		return one + fmod(-one, two);
	else
		return one;
}

void genthread(gendata* g) {
	float x = g->xmin;
	for (unsigned int tx = 0; tx < g->txrez; tx++, x += g->dx) {
		float y = g->s->g.ymin;
		for (unsigned int ty = 0; ty <= g->s->g.yrez; ty++, y += g->dy) {
			float z = eval(g->s->g.eq, x, y);

			if (z < g->zmin) g->zmin = z;
			else if (z > g->zmax) g->zmax = z;

			g->ret.push_back(x);
			g->ret.push_back(y);
			g->ret.push_back(z);
		}
	}
}

void gengraph(state* s) {
	unsigned int numthreads = thread::hardware_concurrency();
	int cpuinfo[4];
	__cpuid(cpuinfo, 1);
	bool HT = (cpuinfo[3] & (1 << 28)) > 0;
	if (HT) numthreads /= 2;

	s->verticies.clear();

	float dx = (s->g.xmax - s->g.xmin) / s->g.xrez;
	float dy = (s->g.ymax - s->g.ymin) / s->g.yrez;
	float xmin = s->g.xmin;
	unsigned int txDelta = s->g.xrez / numthreads;
	unsigned int txLast = (txDelta > 0 ? s->g.xrez % txDelta : s->g.xrez) + txDelta + 1;
	
	vector<thread> threads;
	vector<gendata*> data;
	for (int i = 0; i < numthreads; i++) {
		if (txDelta || i == numthreads - 1) {
			gendata* d = new gendata;

			if (i == numthreads - 1)
				d->txrez = txLast;
			else
				d->txrez = txDelta;

			d->s = s;
			d->dx = dx;
			d->dy = dy;
			d->xmin = xmin;

			data.push_back(d);
			threads.push_back(thread(genthread, data.back()));

			xmin += txDelta * dx;
		}
	}
	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
		s->verticies.insert(s->verticies.end(), data[i]->ret.begin(), data[i]->ret.end());
		data[i]->ret.clear();
	}

	for (unsigned int x = 0; x < s->g.xrez; x++) {
		for (unsigned int y = 0; y < s->g.yrez; y++) {
			GLuint index = x * (s->g.yrez + 1) + y;

			if (!isnan(s->verticies[index * 3 + 2]) &&
				!isinf(s->verticies[index * 3 + 2])) {
				s->indicies.push_back(index);
				s->indicies.push_back(index + 1);
				s->indicies.push_back(index + s->g.yrez + 1);

				s->indicies.push_back(index + 1);
				s->indicies.push_back(index + s->g.yrez + 1);
				s->indicies.push_back(index + s->g.yrez + 2);
			}
		}
	}

	axes[x_min] = s->g.xmin;
	axes[x_max] = s->g.xmax;
	axes[y_min] = s->g.ymin;
	axes[y_max] = s->g.ymax;

	float zmin = FLT_MAX, zmax = -FLT_MAX;
	for (int i = 0; i < threads.size(); i++) {
		if (data[i]->zmin < zmin) zmin = data[i]->zmin;
		if (data[i]->zmax > zmax) zmax = data[i]->zmax;
	}
	if (zmin > 0) zmin = 0;
	if (zmax < 0) zmax = 0;
	axes[z_min] = zmin;
	axes[z_max] = zmax;

	for (gendata* g : data)
		delete g;
}

void kill(state* s) {
	SDL_GL_DeleteContext(s->context);
	SDL_DestroyWindow(s->window);
	SDL_Quit();
}

void loop(state* s) {
	glBindVertexArray(s->VAO);
	while (s->running) {
		
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		mat4 model, view, proj;
		view = lookAt(vec3(5.0f * sin(SDL_GetTicks() / 2000.0f), 5, 5.0f * cos(SDL_GetTicks() / 2000.0f)), vec3(0, 0, 0), vec3(0, 1, 0));
		view = rotate(view, radians(90.0f), vec3(1, 0, 0));
		view = rotate(view, radians(180.0f), vec3(0, 1, 0));
		proj = perspective(radians(75.0f), (GLfloat)s->w / (GLfloat)s->h, 0.1f, 100.0f);


		glUseProgram(s->graphShader);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glUniformMatrix4fv(glGetUniformLocation(s->graphShader, "model"), 1, GL_FALSE, value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(s->graphShader, "view"), 1, GL_FALSE, value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(s->graphShader, "proj"), 1, GL_FALSE, value_ptr(proj));
		glUniform4f(glGetUniformLocation(s->graphShader, "vcolor"), 0.2f, 0.2f, 0.2f, 1.0f);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * s->indicies.size(), s->indicies.size() ? &s->indicies[0] : NULL, GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * s->verticies.size(), s->verticies.size() ? &s->verticies[0] : NULL, GL_STATIC_DRAW);
		
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDrawElements(GL_TRIANGLES, s->indicies.size(), GL_UNSIGNED_INT, (void*)0);


		glUseProgram(s->axisShader);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		glUniformMatrix4fv(glGetUniformLocation(s->axisShader, "model"), 1, GL_FALSE, value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(s->axisShader, "view"), 1, GL_FALSE, value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(s->axisShader, "proj"), 1, GL_FALSE, value_ptr(proj));

		glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_LINES, 0, 6);
		
		glUseProgram(0);

		SDL_Event ev;
		while (SDL_PollEvent(&ev) != 0) {
			switch (ev.type) {
			case SDL_QUIT:
				s->running = false;
				break;
			case SDL_WINDOWEVENT:
				if (ev.window.event == SDL_WINDOWEVENT_RESIZED) {
					s->w = ev.window.data1;
					s->h = ev.window.data2;
					glViewport(0, 0, s->w, s->h);
				}
				break;
			case SDL_KEYDOWN:
				if (ev.key.keysym.sym == SDLK_ESCAPE) {
					s->running = false;
				}
				break;
			}
		}

		SDL_GL_SwapWindow(s->window);
	}
}

void setup(state* s, int w, int h) {
	s->w = w;
	s->h = h;

	s->window = SDL_CreateWindow("3D Grapher", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	assert(s->window);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetSwapInterval(-1);

	s->context = SDL_GL_CreateContext(s->window);
	assert(s->context);

	glewExperimental = GL_TRUE;
	glewInit();

	glEnable(GL_DEPTH_TEST);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, w, h);

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
	
	glGenVertexArrays(1, &s->VAO);
	glGenBuffers(1, &s->VBO);
	glGenBuffers(1, &s->EBO);
	glBindVertexArray(s->VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, s->VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->EBO);

	glBindVertexArray(0);

	s->running = true;
}