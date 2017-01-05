
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
	float z = g->zmin;
	for (int tz = 0; tz < g->tzrez; tz++, z += g->dz) {
		float t = g->s->set.cdom.tmin;
		for (int tt = 0; tt <= g->s->set.cdom.trez; tt++, t += g->dt) {
			float r = eval(g->s->graphs[index]->eq, { { 'z',z },{ 't',t } });

			if (z < g->rmin) g->rmin = r;
			else if (z > g->rmax) g->rmax = r;

			g->ret.push_back(r * cos(t));
			g->ret.push_back(r * sin(t));
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
	glDeleteBuffers(1, &normVBO);
}

void graph::gen() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenBuffers(1, &normVBO);
}

void graph::send() {
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verticies.size(), verticies.size() ? &verticies[0] : NULL, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, normVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * normals.size(), normals.size() ? &normals[0] : NULL, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indicies.size(), indicies.size() ? &indicies[0] : NULL, GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void graph::draw(state* s, mat4 model, mat4 view, mat4 proj) {
	if (dim == s->set.display) {
		glBindVertexArray(VAO);
		{
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);

			mat4 modelviewproj = proj * view * model;

			if (s->set.lighting) {
				s->graph_s_light.use();

				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
				glEnableVertexAttribArray(0);

				glBindBuffer(GL_ARRAY_BUFFER, normVBO);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
				glEnableVertexAttribArray(1);

				glUniformMatrix4fv(s->graph_s_light.getUniform("model"), 1, GL_FALSE, value_ptr(model));
				glUniformMatrix4fv(s->graph_s_light.getUniform("modelviewproj"), 1, GL_FALSE, value_ptr(modelviewproj));

				glUniform4f(s->graph_s_light.getUniform("vcolor"), 0.8f, 0.8f, 0.8f, s->set.graphopacity);
				glUniform3f(s->graph_s_light.getUniform("lightColor"), 1.0f, 1.0f, 1.0f);

				if (s->set.camtype == cam_3d)
					glUniform3f(s->graph_s_light.getUniform("lightPos"), s->c_3d.pos.x, s->c_3d.pos.y, s->c_3d.pos.z);
				else
					glUniform3f(s->graph_s_light.getUniform("lightPos"), s->c_3d_static.pos.x, s->c_3d_static.pos.y, s->c_3d_static.pos.z);
			}
			else {
				s->graph_s.use();

				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
				glEnableVertexAttribArray(0);

				glUniformMatrix4fv(s->graph_s.getUniform("modelviewproj"), 1, GL_FALSE, value_ptr(modelviewproj));

				glUniform4f(s->graph_s.getUniform("vcolor"), 0.8f, 0.8f, 0.8f, s->set.graphopacity);
			}

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glPolygonOffset(1.0f, 0.0f);
			glDrawElements(GL_TRIANGLES, (int)indicies.size(), GL_UNSIGNED_INT, (void*)0);

			if (s->set.wireframe) {
				glDisable(GL_BLEND);

				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glPolygonOffset(0.0f, 0.0f);

				if (s->set.lighting)
					glUniform4f(s->graph_s_light.getUniform("vcolor"), 0.0f, 0.0f, 0.0f, s->set.graphopacity);
				else
					glUniform4f(s->graph_s.getUniform("vcolor"), 0.0f, 0.0f, 0.0f, s->set.graphopacity);

				glDrawElements(GL_TRIANGLES, (int)indicies.size(), GL_UNSIGNED_INT, (void*)0);
			}

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
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

	vec3 norm;
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

				float x1 = verticies[i_index * 3];
				float y1 = verticies[i_index * 3 + 1];
				float z1 = verticies[i_index * 3 + 2];
				float x2 = verticies[(i_index + 1) * 3];
				float y2 = verticies[(i_index + 1) * 3 + 1];
				float z2 = verticies[(i_index + 1) * 3 + 2];
				float x3 = verticies[(i_index + s->set.rdom.yrez + 1) * 3];
				float y3 = verticies[(i_index + s->set.rdom.yrez + 1) * 3 + 1];
				float z3 = verticies[(i_index + s->set.rdom.yrez + 1) * 3 + 2];
				vec3 one(x2 - x1, y2 - y1, z2 - z1);
				vec3 two(x3 - x1, y3 - y1, z3 - z1);
				norm = glm::normalize(cross(two, one));
			}
			normals.push_back(norm);
			if(x == 0)
				normals.push_back(norm);
		}
		normals.push_back(norm);
	}
	normals.push_back(norm);

	for (unsigned int i = 1; i < normals.size() - 1; i++) {
		normals[i] = glm::normalize(normals[i - 1] + normals[i] + normals[i + 1]);
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

	float dz = (s->set.cdom.zmax - s->set.cdom.zmin) / s->set.cdom.zrez;
	float dt = (s->set.cdom.tmax - s->set.cdom.tmin) / s->set.cdom.trez;
	float zmin = s->set.cdom.zmin;
	unsigned int tzDelta = s->set.cdom.zrez / numthreads;
	unsigned int tzLast = s->set.cdom.zrez - (numthreads - 1) * tzDelta + 1;

	vector<thread> threads;
	vector<gendata*> data;
	for (unsigned int i = 0; i < numthreads; i++) {
		if (tzDelta || i == numthreads - 1) {
			gendata* d = new gendata;

			if (i == numthreads - 1)
				d->tzrez = tzLast;
			else
				d->tzrez = tzDelta;

			d->s = s;
			d->dz = dz;
			d->dt = dt;
			d->zmin = zmin;
			d->ID = ID;

			data.push_back(d);
			threads.push_back(thread(genthread, data.back()));

			zmin += tzDelta * dz;
		}
	}
	for (unsigned int i = 0; i < threads.size(); i++) {
		threads[i].join();
		verticies.insert(verticies.end(), data[i]->ret.begin(), data[i]->ret.end());
		data[i]->ret.clear();
	}

	vec3 norm;
	for (int z = 0; z < s->set.cdom.zrez; z++) {
		for (int t = 0; t < s->set.cdom.trez; t++) {
			GLuint i_index = z * (s->set.cdom.trez + 1) + t;

			if (!isnan(verticies[i_index * 3 + 2]) &&
				!isinf(verticies[i_index * 3 + 2])) {
				indicies.push_back(i_index);
				indicies.push_back(i_index + 1);
				indicies.push_back(i_index + s->set.cdom.trez + 1);

				indicies.push_back(i_index + 1);
				indicies.push_back(i_index + s->set.cdom.trez + 1);
				indicies.push_back(i_index + s->set.cdom.trez + 2);

				float x1 = verticies[i_index * 3];
				float y1 = verticies[i_index * 3 + 1];
				float z1 = verticies[i_index * 3 + 2];
				float x2 = verticies[(i_index + 1) * 3];
				float y2 = verticies[(i_index + 1) * 3 + 1];
				float z2 = verticies[(i_index + 1) * 3 + 2];
				float x3 = verticies[(i_index + s->set.cdom.trez + 1) * 3];
				float y3 = verticies[(i_index + s->set.cdom.trez + 1) * 3 + 1];
				float z3 = verticies[(i_index + s->set.cdom.trez + 1) * 3 + 2];
				vec3 one(x2 - x1, y2 - y1, z2 - z1);
				vec3 two(x3 - x1, y3 - y1, z3 - z1);
				norm = glm::normalize(cross(two, one));
			}
			normals.push_back(norm);
			if (z == 0)
				normals.push_back(norm);
		}
		normals.push_back(norm);
	}
	normals.push_back(norm);

	float grmin = FLT_MAX, grmax = -FLT_MAX;
	for (unsigned int i = 0; i < threads.size(); i++) {
		if (data[i]->rmin < grmin) grmin = data[i]->rmin;
		if (data[i]->rmax > grmax) grmax = data[i]->rmax;
	}
	zmin = grmin;
	zmax = grmax;

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
	s->graphs[index]->normals.clear();
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