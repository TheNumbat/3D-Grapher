
#include "exp.h"
#include "main.h"

using namespace std;
 
void loop(state* s);
void setup(state* s, int w, int h);
void kill(state* s);
void gengraph(state* s);

int main(int argc, char** args) {
	//welcome(cout);
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

struct gendata {
	gendata() {
		zmin = FLT_MAX;
		zmax = -FLT_MAX;
	};
	state* s;
	vector<float> ret;
	float zoom, dx, dy, xmin, xmax, ymin, ymax, zmin, zmax;
};
void genthread(gendata* g) {
	for (float x = g->xmin; x < g->xmax; x += g->dx) {
		for (float y = g->ymin; y < g->ymax; y += g->dy) {
			float z = eval(g->s->g.eq, x, y);

			if (z < g->zmin) g->zmin = z;
			else if (z > g->zmax) g->zmax = z;

			g->ret.push_back(x*g->zoom);
			g->ret.push_back(y*g->zoom);
			g->ret.push_back(z*g->zoom);
			g->ret.push_back(0);
			g->ret.push_back(0);
			g->ret.push_back(0);
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
	s->g.zoom = 0.2f;

	float dx = (s->g.xmax - s->g.xmin) / s->g.xrez;
	float dy = (s->g.ymax - s->g.ymin) / s->g.yrez;

	float txDelta = (s->g.xmax - s->g.xmin) / numthreads;
	float txmin = s->g.xmin, txmax = s->g.xmin;
	
	vector<thread> threads;
	gendata* data = new gendata[numthreads];
	for (int i = 0; i < numthreads; i++) {
		txmax += txDelta;
		data[i].s = s;
		data[i].dx = dx;
		data[i].dy = dy;
		data[i].xmin = txmin;
		data[i].xmax = txmax;
		data[i].ymin = s->g.ymin;
		data[i].ymax = s->g.ymax;
		data[i].zoom = s->g.zoom;
		threads.push_back(thread(genthread, &data[i]));
		txmin += txDelta;
	}
	for (int i = 0; i < numthreads; i++) {
		threads[i].join();
		s->verticies.insert(s->verticies.end(), data[i].ret.begin(), data[i].ret.end());
		data[i].ret.clear();
	}

	axes[x_min] = s->g.xmin * s->g.zoom;
	axes[x_max] = s->g.xmax * s->g.zoom;
	axes[y_min] = s->g.ymin * s->g.zoom;
	axes[y_max] = s->g.ymax * s->g.zoom;

	float zmin = FLT_MAX, zmax = -FLT_MAX;
	for (int i = 0; i < numthreads; i++) {
		if (data[i].zmin < zmin) zmin = data[i].zmin;
		if (data[i].zmax > zmax) zmax = data[i].zmax;
	}
	axes[z_min] = zmin * s->g.zoom;
	axes[z_max] = zmax * s->g.zoom;

	delete[] data;
}

void kill(state* s) {
	SDL_GL_DeleteContext(s->context);
	SDL_DestroyWindow(s->window);
	SDL_Quit();
}

void loop(state* s) {
	while (s->running) {
		
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(s->VAO);

		mat4 model, view, proj;
		model = rotate(model, radians(90.0f), vec3(1, 0, 0));
		view = lookAt(vec3(5.0f * sin(SDL_GetTicks() / 1000.0f), 3, 5.0f * cos(SDL_GetTicks() / 1000.0f)), vec3(0, 0, 0), vec3(0, 1, 0));
		proj = perspective(radians(60.0f), (GLfloat)s->w / (GLfloat)s->h, 0.1f, 100.0f);

		glUniformMatrix4fv(glGetUniformLocation(s->shader, "model"), 1, GL_FALSE, value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(s->shader, "view"), 1, GL_FALSE, value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(s->shader, "proj"), 1, GL_FALSE, value_ptr(proj));

		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * s->verticies.size(), s->verticies.size() ? &s->verticies[0] : NULL, GL_STATIC_DRAW);
		glDrawArrays(GL_POINTS, 0, s->verticies.size() / 6);

		glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);
		glDrawArrays(GL_LINES, 0, 6);

		glBindVertexArray(0);

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
		SDL_Delay(10);
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
	glViewport(0, 0, w, h);

	GLuint vert, frag;
	vert = glCreateShader(GL_VERTEX_SHADER);
	frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vert, 1, &vertex, NULL);
	glShaderSource(frag, 1, &fragment, NULL);
	glCompileShader(vert);
	glCompileShader(frag);

	s->shader = glCreateProgram();
	glAttachShader(s->shader, vert);
	glAttachShader(s->shader, frag);
	glLinkProgram(s->shader);
	glUseProgram(s->shader);

	glDeleteShader(vert);
	glDeleteShader(frag);
	
	glGenVertexArrays(1, &s->VAO);
	glGenBuffers(1, &s->VBO);
	glBindVertexArray(s->VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, s->VBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	s->running = true;
}