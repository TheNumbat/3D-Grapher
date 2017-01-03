
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

graph::graph(int id, string s) {
	eq_str = s;
	ID = id;
	zmin = zmax = 0;
}

cyl_graph::cyl_graph(int id, string s) : graph(id, s) {
	dim = dim_3d;
	type = graph_cylindrical;
}

void cyl_graph::genthread(gendata* g) {
	int index = getIndex(g->s, g->ID);
	float r = g->rmin;
	for (int tr = 0; tr < g->trrez; tr++, r += g->dr) {
		float t = g->s->set.cdom.tmin;
		for (int tt = 0; tt <= g->s->set.cdom.trez; tt++, t += g->dt) {
			float z = eval(g->s->graphs[index]->eq, { { 'r',r },{ 't',t } });

			if (z < g->zmin) g->zmin = z;
			else if (z > g->zmax) g->zmax = z;

			g->ret.push_back(r * std::cos(t));
			g->ret.push_back(r * std::sin(t));
			g->ret.push_back(z);
		}
	}
}

fxy_graph::fxy_graph(int id, string s) : graph(id, s) {
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

void updateAxes(state* s) {
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

	s->set.rdom.zmin = zmin;
	s->set.rdom.zmax = zmax;
	axes[x_min] = s->set.rdom.xmin;
	axes[y_min] = s->set.rdom.ymin;
	axes[z_min] = s->set.rdom.zmin;
	axes[x_max] = s->set.rdom.xmax;
	axes[y_max] = s->set.rdom.ymax;
	axes[z_max] = s->set.rdom.zmax;

	if (axes[x_min] > 0) axes[x_min] = 0;
	if (axes[y_min] > 0) axes[y_min] = 0;
	if (axes[z_min] > 0) axes[z_min] = 0;
	if (axes[x_max] < 0) axes[x_max] = 0;
	if (axes[y_max] < 0) axes[y_max] = 0;
	if (axes[z_max] < 0) axes[z_max] = 0;

	s->c_3d_static.scale = std::max(s->set.rdom.ymax - s->set.rdom.ymin, s->set.rdom.xmax - s->set.rdom.xmin);
	s->c_3d_static.pos.x = (s->set.rdom.xmax + s->set.rdom.xmin) / 2;
	s->c_3d_static.pos.z = (s->set.rdom.ymax + s->set.rdom.ymin) / -2;

	s->c_3d.pos.x = (s->set.rdom.xmax + s->set.rdom.xmin) / 2;
	s->c_3d.pos.z = (s->set.rdom.ymax + s->set.rdom.ymin) / -2;

	glBindVertexArray(s->axisVAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, s->axisVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);
	}
}

void fxy_graph::genthread(gendata* g) {
	int index = getIndex(g->s, g->ID);
	float x = g->xmin;
	for (int tx = 0; tx < g->txrez; tx++, x += g->dx) {
		float y = g->s->set.rdom.ymin;
		for (int ty = 0; ty <= g->s->set.rdom.yrez; ty++, y += g->dy) {
			float z = eval(g->s->graphs[index]->eq, { { 'x',x },{ 'y',y } });

			if (z < g->zmin) g->zmin = z;
			else if (z > g->zmax) g->zmax = z;

			g->ret.push_back(x);
			g->ret.push_back(y);
			g->ret.push_back(z);
		}
	}
}

void graph::normalize(state* s) {
	if (s->set.axisnormalization) {
		for (unsigned int i = 0; i < verticies.size(); i += 3) {
			verticies[i] /= (s->set.rdom.xmax - s->set.rdom.xmin) / 20;
			verticies[i + 1] /= (s->set.rdom.ymax - s->set.rdom.ymin) / 20;
			verticies[i + 2] /= (zmax - zmin) / 20;
		}
		zmin = -10;
		zmax = 10;
	}
}

void fxy_graph::generate(state* s) {
	unsigned int numthreads = thread::hardware_concurrency();
	int cpuinfo[4];
	__cpuid(cpuinfo, 1);
	bool HT = (cpuinfo[3] & (1 << 28)) > 0;
	if (HT) numthreads /= 2;

	float dx = (s->set.rdom.xmax - s->set.rdom.xmin) / s->set.rdom.xrez;
	float dy = (s->set.rdom.ymax - s->set.rdom.ymin) / s->set.rdom.yrez;
	float xmin = s->set.rdom.xmin;
	unsigned int txDelta = s->set.rdom.xrez / numthreads;
	unsigned int txLast = s->set.rdom.xrez - (numthreads - 1) * txDelta + 1;

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
			d->ID = ID;

			data.push_back(d);
			threads.push_back(thread(genthread, data.back()));

			xmin += txDelta * dx;
		}
	}
	for (unsigned int i = 0; i < threads.size(); i++) {
		threads[i].join();
		verticies.insert(verticies.end(), data[i]->ret.begin(), data[i]->ret.end());
		data[i]->ret.clear();
	}

	for (int x = 0; x < s->set.rdom.xrez; x++) {
		for (int y = 0; y < s->set.rdom.yrez; y++) {
			GLuint i_index = x * (s->set.rdom.yrez + 1) + y;

			if (!isnan(verticies[i_index * 3 + 2]) &&
				!isinf(verticies[i_index * 3 + 2])) {
				indicies.push_back(i_index);
				indicies.push_back(i_index + 1);
				indicies.push_back(i_index + s->set.rdom.yrez + 1);

				indicies.push_back(i_index + 1);
				indicies.push_back(i_index + s->set.rdom.yrez + 1);
				indicies.push_back(i_index + s->set.rdom.yrez + 2);
			}
		}
	}

	float gzmin = FLT_MAX, gzmax = -FLT_MAX;
	for (unsigned int i = 0; i < threads.size(); i++) {
		if (data[i]->zmin < gzmin) gzmin = data[i]->zmin;
		if (data[i]->zmax > gzmax) gzmax = data[i]->zmax;
	}
	zmin = gzmin;
	zmax = gzmax;

	normalize(s);

	for (gendata* g : data)
		delete g;
}

void cyl_graph::generate(state* s) {
	unsigned int numthreads = thread::hardware_concurrency();
	int cpuinfo[4];
	__cpuid(cpuinfo, 1);
	bool HT = (cpuinfo[3] & (1 << 28)) > 0;
	if (HT) numthreads /= 2;

	float dr = (s->set.cdom.rmax - s->set.cdom.rmin) / s->set.cdom.rrez;
	float dt = (s->set.cdom.tmax - s->set.cdom.tmin) / s->set.cdom.trez;
	float rmin = s->set.cdom.rmin;
	unsigned int trDelta = s->set.cdom.rrez / numthreads;
	unsigned int trLast = s->set.cdom.rrez - (numthreads - 1) * trDelta + 1;

	vector<thread> threads;
	vector<gendata*> data;
	for (unsigned int i = 0; i < numthreads; i++) {
		if (trDelta || i == numthreads - 1) {
			gendata* d = new gendata;

			if (i == numthreads - 1)
				d->trrez = trLast;
			else
				d->trrez = trDelta;

			d->s = s;
			d->dr = dr;
			d->dt = dt;
			d->rmin = rmin;
			d->ID = ID;

			data.push_back(d);
			threads.push_back(thread(genthread, data.back()));

			rmin += trDelta * dr;
		}
	}
	for (unsigned int i = 0; i < threads.size(); i++) {
		threads[i].join();
		verticies.insert(verticies.end(), data[i]->ret.begin(), data[i]->ret.end());
		data[i]->ret.clear();
	}

	for (int x = 0; x < s->set.rdom.xrez; x++) {
		for (int y = 0; y < s->set.rdom.yrez; y++) {
			GLuint i_index = x * (s->set.rdom.yrez + 1) + y;

			if (!isnan(verticies[i_index * 3 + 2]) &&
				!isinf(verticies[i_index * 3 + 2])) {
				indicies.push_back(i_index);
				indicies.push_back(i_index + 1);
				indicies.push_back(i_index + s->set.rdom.yrez + 1);

				indicies.push_back(i_index + 1);
				indicies.push_back(i_index + s->set.rdom.yrez + 1);
				indicies.push_back(i_index + s->set.rdom.yrez + 2);
			}
		}
	}

	float gzmin = FLT_MAX, gzmax = -FLT_MAX;
	for (unsigned int i = 0; i < threads.size(); i++) {
		if (data[i]->zmin < gzmin) gzmin = data[i]->zmin;
		if (data[i]->zmax > gzmax) gzmax = data[i]->zmax;
	}
	zmin = gzmin;
	zmax = gzmax;

	normalize(s);

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
	s->graphs[index]->generate(s);
	Uint64 end = SDL_GetPerformanceCounter();
	cout << "time: " << (float)(end - start) / SDL_GetPerformanceFrequency() << endl;

	updateAxes(s);
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
		updateAxes(s);
}