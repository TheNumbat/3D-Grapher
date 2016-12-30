
#include "graph.h"
#include "state.h"

#include <thread>
#include <limits>
#include <vector>
#include <algorithm>

using namespace std;

GLfloat axes[] = {
	-10.0f,  0.0f ,  0.0f ,		1.0f, 0.0f, 0.0f,
	10.0f,  0.0f ,  0.0f ,		1.0f, 0.0f, 0.0f,

	0.0f , -10.0f,  0.0f ,		0.0f, 1.0f, 0.0f,
	0.0f ,  10.0f,  0.0f ,		0.0f, 1.0f, 0.0f,

	0.0f ,  0.0f , -10.0f,		0.0f, 0.0f, 1.0f,
	0.0f ,  0.0f ,  10.0f,		0.0f, 0.0f, 1.0f
};

struct gendata {
	gendata() {
		zmin = FLT_MAX;
		zmax = -FLT_MAX;
	};
	state* s;
	vector<float> ret;
	float zmin, zmax, xmin, dx, dy;
	int txrez;
};

graph::graph(int id, string s, float xmi, float xma, float ymi, float yma, unsigned int xr, unsigned int yr) {
	eq_str = s;
	ID = id;
	zmin = zmax = 0;
	dim = dim_3d;
	type = graph_func;
}

graph::~graph() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void graph::gen() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
}

void graph::send() {
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verticies.size(), verticies.size() ? &verticies[0] : NULL, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indicies.size(), indicies.size() ? &indicies[0] : NULL, GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void graph::draw(state* s, mat4& modelviewproj) {
	if (dim == s->set.display) {
		glBindVertexArray(VAO);
		{
			s->graph_s.use();

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);

			glUniformMatrix4fv(s->graph_s.getUniform("modelviewproj"), 1, GL_FALSE, value_ptr(modelviewproj));

			glEnable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glPolygonOffset(1.0f, 0.0f);

			glUniform4f(s->graph_s.getUniform("vcolor"), 0.8f, 0.8f, 0.8f, s->set.graphopacity);
			glDrawElements(GL_TRIANGLES, (int)indicies.size(), GL_UNSIGNED_INT, (void*)0);

			if (s->set.wireframe) {
				glDisable(GL_BLEND);

				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glPolygonOffset(0.0f, 0.0f);

				glUniform4f(s->graph_s.getUniform("vcolor"), 0.2f, 0.2f, 0.2f, s->set.graphopacity);
				glDrawElements(GL_TRIANGLES, (int)indicies.size(), GL_UNSIGNED_INT, (void*)0);
			}

			glDisableVertexAttribArray(0);
		}
		glBindVertexArray(0);
	}
}

void sendAxes(state* s) {
	float zmin = FLT_MAX, zmax = -FLT_MAX;
	
	if (s->graphs.size()) {
		for (graph* g : s->graphs) {
			if (g->zmin < zmin) zmin = g->zmin;
			if (g->zmax > zmax) zmax = g->zmax;
		}
		if (zmin > 0) zmin = 0;
		if (zmax < 0) zmax = 0;
	}
	else {
		zmin = -10;
		zmax = 10;
	}

	s->set.zmin = zmin;
	s->set.zmax = zmax;
	axes[x_min] = s->set.xmin;
	axes[y_min] = s->set.ymin;
	axes[z_min] = s->set.zmin;
	axes[x_max] = s->set.xmax;
	axes[y_max] = s->set.ymax;
	axes[z_max] = s->set.zmax;

	glBindVertexArray(s->axisVAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, s->axisVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);
	}
}

void genthread(gendata* g, int index) {
	float x = g->xmin;
	for (int tx = 0; tx < g->txrez; tx++, x += g->dx) {
		float y = g->s->set.ymin;
		for (int ty = 0; ty <= g->s->set.yrez; ty++, y += g->dy) {
			float z = eval(g->s->graphs[index]->eq, x, y);

			if (z < g->zmin) g->zmin = z;
			else if (z > g->zmax) g->zmax = z;

			g->ret.push_back(x);
			g->ret.push_back(y);
			g->ret.push_back(z);
		}
	}
}

void gengraph(state* s, int index) {
	unsigned int numthreads = thread::hardware_concurrency();
	int cpuinfo[4];
	__cpuid(cpuinfo, 1);
	bool HT = (cpuinfo[3] & (1 << 28)) > 0;
	if (HT) numthreads /= 2;

	float dx = (s->set.xmax - s->set.xmin) / s->set.xrez;
	float dy = (s->set.ymax - s->set.ymin) / s->set.yrez;
	float xmin = s->set.xmin;
	unsigned int txDelta = s->set.xrez / numthreads;
	unsigned int txLast = s->set.xrez - (numthreads - 1) * txDelta + 1;

	vector<thread> threads;
	vector<gendata*> data;
	for (unsigned int i = 0; i < numthreads; i++) {
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
			threads.push_back(thread(genthread, data.back(), index));

			xmin += txDelta * dx;
		}
	}
	for (unsigned int i = 0; i < threads.size(); i++) {
		threads[i].join();
		s->graphs[index]->verticies.insert(s->graphs[index]->verticies.end(), data[i]->ret.begin(), data[i]->ret.end());
		data[i]->ret.clear();
	}

	for (int x = 0; x < s->set.xrez; x++) {
		for (int y = 0; y < s->set.yrez; y++) {
			GLuint i_index = x * (s->set.yrez + 1) + y;

			if (!isnan(s->graphs[index]->verticies[i_index * 3 + 2]) &&
				!isinf(s->graphs[index]->verticies[i_index * 3 + 2])) {
				s->graphs[index]->indicies.push_back(i_index);
				s->graphs[index]->indicies.push_back(i_index + 1);
				s->graphs[index]->indicies.push_back(i_index + s->set.yrez + 1);

				s->graphs[index]->indicies.push_back(i_index + 1);
				s->graphs[index]->indicies.push_back(i_index + s->set.yrez + 1);
				s->graphs[index]->indicies.push_back(i_index + s->set.yrez + 2);
			}
		}
	}

	float zmin = FLT_MAX, zmax = -FLT_MAX;
	for (unsigned int i = 0; i < threads.size(); i++) {
		if (data[i]->zmin < zmin) zmin = data[i]->zmin;
		if (data[i]->zmax > zmax) zmax = data[i]->zmax;
	}
	s->graphs[index]->zmin = zmin;
	s->graphs[index]->zmax = zmax;

	if (s->set.axisnormalization) {
		for (unsigned int i = 0; i < s->graphs[index]->verticies.size(); i += 3) {
			s->graphs[index]->verticies[i] /= (s->set.xmax - s->set.xmin) / 20;
			s->graphs[index]->verticies[i + 1] /= (s->set.ymax - s->set.ymin) / 20;
			s->graphs[index]->verticies[i + 2] /= (s->graphs[index]->zmax - s->graphs[index]->zmin) / 20;
		}
		s->graphs[index]->zmin = -10;
		s->graphs[index]->zmax = 10;
	}

	for (gendata* g : data)
		delete g;
}

void regengraph(state* s, int index) {
	
	vector<op> new_eq;

	if (!in(s->graphs[index]->eq_str, new_eq)) {
		return;
	}
	s->graphs[index]->indicies.clear();
	s->graphs[index]->verticies.clear();
	s->graphs[index]->eq = new_eq;

	printeq(cout, s->graphs[index]->eq);

	Uint64 start = SDL_GetPerformanceCounter();
	gengraph(s, index);
	Uint64 end = SDL_GetPerformanceCounter();
	cout << "time: " << (float)(end - start) / SDL_GetPerformanceFrequency() << endl;

	sendAxes(s);
	s->graphs[index]->send();
}

int getIndex(state* s, int ID) {
	auto entry = find_if(s->graphs.begin(), s->graphs.end(), [ID](graph* g) -> bool {return g->ID == ID;});
	if (entry == s->graphs.end()) {
		return -1;
	}
	else {
		int pos = entry - s->graphs.begin();
		return pos;
	}
}

void regenall(state* s) {
	bool gen = false;
	for (int i = 0; i < (int)s->graphs.size(); i++) {
		if (s->graphs[i]->verticies.size()) {
			regengraph(s, i);
			gen = true;
		}
	}
	if (!gen)
		sendAxes(s);
}