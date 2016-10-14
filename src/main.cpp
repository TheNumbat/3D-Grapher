
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

	gengraph(&st);

	loop(&st);

	kill(&st);

	return 0;
}

void gengraph(state* s) {
	float dx = (s->g.xmax - s->g.xmin) / s->g.xrez;
	float dy = (s->g.ymax - s->g.ymin) / s->g.yrez;

	int xc = 0;
	for (float x = s->g.xmin; x <= s->g.xmax; x += dx, xc++) {
		s->g.data.push_back(vector<float>());
		for (float y = s->g.ymin; y <= s->g.ymax; y += dy) {
			s->g.data[xc].push_back(eval(s->g.eq, x, y));
		}
	}

	for (auto v : s->g.data) {
		for (float f : v) {
			cout << f << " ";
		}
		cout << endl;
	}
}

void kill(state* s) {
	SDL_GL_DeleteContext(s->context);
	SDL_DestroyWindow(s->window);
	SDL_Quit();
}

void loop(state* s) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	while (s->running) {

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4 model, view, proj;
		view = lookAt(vec3(3, 3, -3), vec3(0, 0, 0), vec3(0, 1, 0));
		proj = perspective(45.0f, (GLfloat)s->w / (GLfloat)s->h, 0.1f, 100.0f);

		glUniformMatrix4fv(glGetUniformLocation(s->shader, "model"), 1, GL_FALSE, value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(s->shader, "view"), 1, GL_FALSE, value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(s->shader, "proj"), 1, GL_FALSE, value_ptr(proj));

		glBindVertexArray(s->VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		SDL_GL_SwapWindow(s->window);

		SDL_Event ev;
		while (SDL_PollEvent(&ev) != 0) {
			switch (ev.type) {
			case SDL_QUIT:
				s->running = false;
				break;
			}
		}
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
	glBindVertexArray(s->VAO);

	glGenBuffers(1, &s->VBO);
	glBindBuffer(GL_ARRAY_BUFFER, s->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	s->running = true;
}