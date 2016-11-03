
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
		// Transparency, blending, maybe sorting
		// Lighting
		// Multiple graphs
	// More features
		// Partials
		// Double integrals?
		// Vector feilds
		// Probably a ton more stuff I can't think of right now
		// 2D and 4D graphs

void loop(state* s);
void setup(state* s, int w, int h);
void kill(state* s);
void gengraph(state* s);

int main(int argc, char** args) {
	
	state st;
	setup(&st, 1280, 720);

	string exp;
	stringstream ss;

	st.g.xmin = -20;
	st.g.xmax = 20;
	st.g.ymin = -20;
	st.g.ymax = 20;
	st.g.xrez = 250;
	st.g.yrez = 250;
	exp = "sin(x)*sin(y)";

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
	unsigned int txLast = s->g.xrez - (numthreads - 1) * txDelta + 1;
	
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
	glBindVertexArray(s->VAO);
	glDeleteBuffers(1, &s->axisVBO);
	glDeleteBuffers(1, &s->graphVBO);
	glDeleteBuffers(1, &s->EBO);
	SDL_GL_DeleteContext(s->context);
	SDL_DestroyWindow(s->window);
	SDL_Quit();
}

void loop(state* s) {

	int mx = s->w / 2, my = s->h / 2;
	const unsigned char* keys = SDL_GetKeyboardState(NULL);

	glBindVertexArray(s->VAO);

	glBindBuffer(GL_ARRAY_BUFFER, s->graphVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->EBO);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * s->indicies.size(), s->indicies.size() ? &s->indicies[0] : NULL, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * s->verticies.size(), s->verticies.size() ? &s->verticies[0] : NULL, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, s->axisVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);

	while (s->running) {
		
		Uint64 start = SDL_GetPerformanceCounter();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		mat4 model, view, proj;
		view = getView(s->c);
		proj = perspective(radians(s->c.fov), (GLfloat)s->w / (GLfloat)s->h, 0.1f, 1000.0f);

		glUseProgram(s->graphShader);
		{
			glBindBuffer(GL_ARRAY_BUFFER, s->graphVBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->EBO);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);

			glUniformMatrix4fv(glGetUniformLocation(s->graphShader, "model"), 1, GL_FALSE, value_ptr(model));
			glUniformMatrix4fv(glGetUniformLocation(s->graphShader, "view"), 1, GL_FALSE, value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(s->graphShader, "proj"), 1, GL_FALSE, value_ptr(proj));

			glEnable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glPolygonOffset(0.0f, 0.0f);
			
			glUniform4f(glGetUniformLocation(s->graphShader, "vcolor"), 0.8f, 0.8f, 0.8f, 1.0f);
			glDrawElements(GL_TRIANGLES, s->indicies.size(), GL_UNSIGNED_INT, (void*)0);

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glPolygonOffset(-1.0f, -1.0f);
			
			glUniform4f(glGetUniformLocation(s->graphShader, "vcolor"), 0.2f, 0.2f, 0.2f, 1.0f);
			glDrawElements(GL_TRIANGLES, s->indicies.size(), GL_UNSIGNED_INT, (void*)0);
		}

		glUseProgram(s->axisShader);
		{
			glBindBuffer(GL_ARRAY_BUFFER, s->axisVBO);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
			glEnableVertexAttribArray(1);

			glUniformMatrix4fv(glGetUniformLocation(s->axisShader, "model"), 1, GL_FALSE, value_ptr(model));
			glUniformMatrix4fv(glGetUniformLocation(s->axisShader, "view"), 1, GL_FALSE, value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(s->axisShader, "proj"), 1, GL_FALSE, value_ptr(proj));

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glPolygonOffset(0.0f, 0.0f);
			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);

			glDrawArrays(GL_LINES, 0, 6);
		}

		glUseProgram(0);

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
					glViewport(0, 0, s->w, s->h);
				}
				SDL_CaptureMouse(SDL_TRUE);
				SDL_ShowCursor(0);
				SDL_SetRelativeMouseMode(SDL_TRUE);
				break;
			}
			case SDL_MOUSEMOTION: {
				float sens = 0.1f;
				float dx = (ev.motion.x - mx) * sens;
				float dy = (ev.motion.y - my) * sens;
				mx = ev.motion.x;
				my = ev.motion.y;
				s->c.yaw += dx;
				s->c.pitch -= dy;
				if (s->c.yaw > 360.0f) s->c.yaw = 0.0f;
				else if (s->c.yaw < 0.0f) s->c.yaw = 360.0f;
				if (s->c.pitch > 89.9f) s->c.pitch = 89.9f;
				else if (s->c.pitch < -89.9f) s->c.pitch = -89.9f;
				updoot(s->c);
				break;
			}
			case SDL_MOUSEWHEEL: {
				s->c.fov -= ev.wheel.y;
				if (s->c.fov > 179.0f) s->c.fov = 179.0f;
				else if (s->c.fov < 1.0f) s->c.fov = 1.0f;
				break;
			}
			case SDL_KEYDOWN: {
				switch (ev.key.keysym.sym) {
				case SDLK_ESCAPE:
					s->running = false;
					break;
				}
				break;
			}
			}
		}
		float dT = (SDL_GetTicks() - s->c.lastUpdate) / 1000.0f;
		s->c.lastUpdate = SDL_GetTicks();
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
		
		SDL_GL_SwapWindow(s->window);

		Uint64 end = SDL_GetPerformanceCounter();
		cout << "frame: " << (end - start) / (1000.0f  * SDL_GetPerformanceFrequency()) << "ms" << endl;
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
	SDL_GL_SetSwapInterval(1);
	SDL_CaptureMouse(SDL_TRUE);
	SDL_ShowCursor(0);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	s->context = SDL_GL_CreateContext(s->window);
	assert(s->context);

	glewExperimental = GL_TRUE;
	glewInit();

	glEnable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_DEPTH_TEST);
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
	glGenBuffers(1, &s->axisVBO);
	glGenBuffers(1, &s->graphVBO);
	glGenBuffers(1, &s->EBO);
	glBindVertexArray(s->VAO);

	glBindVertexArray(0);

	s->c = defaultCam();
	s->running = true;
}