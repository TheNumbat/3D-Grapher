
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

void regengraph(state* s, int index) {

	vector<op> new_eq;

	try { in(s->graphs[index]->eq_str, new_eq); }
	catch (runtime_error e) {
		s->ui->error = e.what();
		s->ui->errorShown = true;
		s->ev.current = in_help_or_err;
		return;
	}

	s->graphs[index]->eq = new_eq;
	printeq(cout, s->graphs[index]->eq);

	Uint64 start = SDL_GetPerformanceCounter();
	s->graphs[index]->generate(s);
	//s->graphs[index]->generateIndiciesAndNormals(s);
	Uint64 end = SDL_GetPerformanceCounter();
	cout << "time: " << (float)(end - start) / SDL_GetPerformanceFrequency() << endl;

	updateAxes(s);
	s->graphs[index]->send();
}

int getIndex(state* s, int ID) {
	auto entry = find_if(s->graphs.begin(), s->graphs.end(), [ID](graph* g) -> bool {return g->ID == ID; });
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
		regengraph(s, i);
		if (s->graphs[i]->verticies.size()) {
			gen = true;
		}
	}
	if (gen) {
		updateAxes(s);
		//resetCam(s);
	}
}

void updateAxes(state* s) {
	float zmin = FLT_MAX, zmax = -FLT_MAX;

	if (s->graphs.size()) {
		for (graph* g : s->graphs) {
			if (g->zmin < zmin) zmin = g->zmin;
			if (g->zmax > zmax) zmax = g->zmax;
			if (!g->verticies.size()) {
				zmin = -10;
				zmax = 10;
			}
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

	glBindVertexArray(s->axisVAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, s->axisVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);
	}
}

void resetCam(state* s) {
	s->c_3d_static.radius = std::max(s->set.rdom.ymax - s->set.rdom.ymin, s->set.rdom.xmax - s->set.rdom.xmin);
	s->c_3d_static.lookingAt.x = (s->set.rdom.xmax + s->set.rdom.xmin) / 2;
	s->c_3d_static.lookingAt.z = (s->set.rdom.ymax + s->set.rdom.ymin) / -2;
	s->c_3d_static.updatePos();

	s->c_3d.pos.x = (s->set.rdom.xmax + s->set.rdom.xmin) / 2;
	s->c_3d.pos.z = (s->set.rdom.ymax + s->set.rdom.ymin) / -2;
	s->c_3d.updateFront();
}

graph::graph(int id, string s) {
	eq_str = s;
	ID = id;
	zmin = zmax = 0;
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
			glUniform1f(s->graph_s_light.getUniform("ambientStrength"), s->set.ambientLighting);

			if (s->set.camtype == cam_3d) {

				glUniform3f(s->graph_s_light.getUniform("lightPos"), s->c_3d.pos.x, s->c_3d.pos.y, s->c_3d.pos.z);
			}
			else {
				glUniform3f(s->graph_s_light.getUniform("lightPos"), s->c_3d_static.pos.x, s->c_3d_static.pos.y, s->c_3d_static.pos.z);
			}
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

void graph::generateIndiciesAndNormals(state* s) {
	indicies.clear();
	normals.clear();

	vec3 norm;
	int x_max;
	int y_max;
	if (type == graph_func) {
		x_max = s->set.rdom.xrez;
		y_max = s->set.rdom.yrez;
	}
	else if (type == graph_cylindrical) {
		x_max = s->set.cdom.trez;
		y_max = s->set.cdom.zrez;
	}
	else if (type == graph_spherical) {
		x_max = s->set.sdom.prez;
		y_max = s->set.sdom.trez;
	}

	for (int x = 0; x < x_max; x++) {
		for (int y = 0; y < y_max; y++) {
			GLuint i_index = x * (y_max + 1) + y;

			if (!isnan(verticies[i_index * 3 + 2]) &&
				!isinf(verticies[i_index * 3 + 2])) {
				indicies.push_back(i_index);
				indicies.push_back(i_index + 1);
				indicies.push_back(i_index + y_max + 1);

				indicies.push_back(i_index + 1);
				indicies.push_back(i_index + y_max + 1);
				indicies.push_back(i_index + y_max + 2);

				float x1 = verticies[i_index * 3];
				float y1 = verticies[i_index * 3 + 1];
				float z1 = verticies[i_index * 3 + 2];
				float x2 = verticies[(i_index + 1) * 3];
				float y2 = verticies[(i_index + 1) * 3 + 1];
				float z2 = verticies[(i_index + 1) * 3 + 2];
				float x3 = verticies[(i_index + y_max + 1) * 3];
				float y3 = verticies[(i_index + y_max + 1) * 3 + 1];
				float z3 = verticies[(i_index + y_max + 1) * 3 + 2];
				vec3 one(x2 - x1, y2 - y1, z2 - z1);
				vec3 two(x3 - x1, y3 - y1, z3 - z1);
				norm = glm::normalize(cross(two, one));
			}
			normals.push_back(norm);
			if (x == 0)
				normals.push_back(norm);
		}
		normals.push_back(norm);
	}
	normals.push_back(norm);
}

fxy_graph::fxy_graph(int id, string s) : graph(id, s) {
	type = graph_func;
}

void fxy_graph::genthread(gendata* g) {
	int index = getIndex(g->s, g->ID);
	float x = g->xmin;
	for (int tx = 0; tx < g->txrez; tx++, x += g->dx) {
		float y = g->s->set.rdom.ymin;
		for (int ty = 0; ty <= g->s->set.rdom.yrez; ty++, y += g->dy) {
			float z;
			try { z = eval(g->s->graphs[index]->eq, { { 'x',x },{ 'y',y } }); }
			catch (runtime_error e) {
				g->s->ui->error = e.what();
				g->s->ui->errorShown = true;
				g->s->ev.current = in_help_or_err;
				g->success = false;
				return;
			}

			if (z < g->zmin) g->zmin = z;
			else if (z > g->zmax) g->zmax = z;

			g->ret.push_back(x);
			g->ret.push_back(y);
			g->ret.push_back(z);
		}
	}
	g->success = true;
}

void fxy_graph::generate(state* s) {
	unsigned int numthreads = thread::hardware_concurrency();
#ifdef _MSC_VER
	int cpuinfo[4];
	__cpuid(cpuinfo, 1);
	bool HT = (cpuinfo[3] & (1 << 28)) > 0;
	if (HT) numthreads /= 2;
#endif

	float dx = (s->set.rdom.xmax - s->set.rdom.xmin) / s->set.rdom.xrez;
	float dy = (s->set.rdom.ymax - s->set.rdom.ymin) / s->set.rdom.yrez;
	float xmin = s->set.rdom.xmin;
	unsigned int txDelta = s->set.rdom.xrez / numthreads;
	unsigned int txLast = s->set.rdom.xrez - (numthreads - 1) * txDelta + 1;

	verticies.clear();

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
	bool success = true;
	for (unsigned int i = 0; i < threads.size(); i++) {
		threads[i].join();
		if (success) {
			success = data[i]->success;
			verticies.insert(verticies.end(), data[i]->ret.begin(), data[i]->ret.end());
			data[i]->ret.clear();
		}
	}

	if (success) {
		float gzmin = FLT_MAX, gzmax = -FLT_MAX;
		for (unsigned int i = 0; i < threads.size(); i++) {
			if (data[i]->zmin < gzmin) gzmin = data[i]->zmin;
			if (data[i]->zmax > gzmax) gzmax = data[i]->zmax;
		}
		zmin = gzmin;
		zmax = gzmax;

		normalize(s);
		generateIndiciesAndNormals(s);
	}

	for (gendata* g : data)
		delete g;
}

cyl_graph::cyl_graph(int id, string s) : graph(id, s) {
	type = graph_cylindrical;
}

void cyl_graph::genthread(gendata* g) {
	int index = getIndex(g->s, g->ID);
	float z = g->zmin;
	for (int tz = 0; tz < g->tzrez; tz++, z += g->dz) {
		float t = g->s->set.cdom.tmin;
		for (int tt = 0; tt <= g->s->set.cdom.trez; tt++, t += g->dt) {
			float r;
			try { r = eval(g->s->graphs[index]->eq, { { 'z',z },{ 't',t } }); }
			catch (runtime_error e) {
				g->s->ui->error = e.what();
				g->s->ui->errorShown = true;
				g->s->ev.current = in_help_or_err;
				g->success = false;
				return;
			}

			if (z < g->gzmin) g->gzmin = z;
			else if (z > g->gzmax) g->gzmax = z;

			g->ret.push_back(r * cos(t));
			g->ret.push_back(r * sin(t));
			g->ret.push_back(z);
		}
	}
	g->success = true;
}

void cyl_graph::generate(state* s) {
	unsigned int numthreads = thread::hardware_concurrency();
#ifdef _MSC_VER
	int cpuinfo[4];
	__cpuid(cpuinfo, 1);
	bool HT = (cpuinfo[3] & (1 << 28)) > 0;
	if (HT) numthreads /= 2;
#endif

	float dz = (s->set.cdom.zmax - s->set.cdom.zmin) / s->set.cdom.zrez;
	float dt = (s->set.cdom.tmax - s->set.cdom.tmin) / s->set.cdom.trez;
	float zmin = s->set.cdom.zmin;
	unsigned int tzDelta = s->set.cdom.zrez / numthreads;
	unsigned int tzLast = s->set.cdom.zrez - (numthreads - 1) * tzDelta + 1;

	verticies.clear();

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
	bool success = true;
	for (unsigned int i = 0; i < threads.size(); i++) {
		threads[i].join();
		if (success) {
			success = data[i]->success;
			verticies.insert(verticies.end(), data[i]->ret.begin(), data[i]->ret.end());
			data[i]->ret.clear();
		}
	}

	if (success) {
		float gzmin = FLT_MAX, gzmax = -FLT_MAX;
		for (unsigned int i = 0; i < threads.size(); i++) {
			if (data[i]->gzmin < gzmin) gzmin = data[i]->gzmin;
			if (data[i]->gzmax > gzmax) gzmax = data[i]->gzmax;
		}
		zmin = gzmin;
		zmax = gzmax;

		normalize(s);
		generateIndiciesAndNormals(s);
	}

	for (gendata* g : data)
		delete g;
}

spr_graph::spr_graph(int id, string s) : graph(id, s) {
	type = graph_spherical;
}

void spr_graph::genthread(gendata* g) {
	int index = getIndex(g->s, g->ID);
	float p = g->pmin;
	for (int tp = 0; tp < g->tprez; tp++, p += g->dp) {
		float t = g->s->set.sdom.tmin;
		for (int tt = 0; tt <= g->s->set.sdom.trez; tt++, t += g->dt) {
			float r;
			try { r = eval(g->s->graphs[index]->eq, { { 't',t },{ 'p',p } }); }
			catch (runtime_error e) {
				g->s->ui->error = e.what();
				g->s->ui->errorShown = true;
				g->s->ev.current = in_help_or_err;
				g->success = false;
				return;
			}

			if (r * cos(p) < g->zmin) g->zmin = r * cos(p);
			else if (r * cos(p) > g->zmax) g->zmax = r * cos(p);

			g->ret.push_back(r * cos(t) * sin(p));
			g->ret.push_back(r * sin(t) * sin(p));
			g->ret.push_back(r * cos(p));
		}
	}
	g->success = true;
}

void spr_graph::generate(state* s) {
	unsigned int numthreads = thread::hardware_concurrency();
#ifdef _MSC_VER
	int cpuinfo[4];
	__cpuid(cpuinfo, 1);
	bool HT = (cpuinfo[3] & (1 << 28)) > 0;
	if (HT) numthreads /= 2;
#endif

	float dt = (s->set.sdom.tmax - s->set.sdom.tmin) / s->set.sdom.trez;
	float dp = (s->set.sdom.pmax - s->set.sdom.pmin) / s->set.sdom.prez;
	float pmin = s->set.sdom.pmin;
	unsigned int tpDelta = s->set.sdom.prez / numthreads;
	unsigned int tpLast = s->set.sdom.prez - (numthreads - 1) * tpDelta + 1;

	verticies.clear();

	vector<thread> threads;
	vector<gendata*> data;
	for (unsigned int i = 0; i < numthreads; i++) {
		if (tpDelta || i == numthreads - 1) {
			gendata* d = new gendata;

			if (i == numthreads - 1)
				d->tprez = tpLast;
			else
				d->tprez = tpDelta;

			d->s = s;
			d->dt = dt;
			d->dp = dp;
			d->pmin = pmin;
			d->ID = ID;

			data.push_back(d);
			threads.push_back(thread(genthread, data.back()));

			pmin += tpDelta * dp;
		}
	}
	bool success = true;
	for (unsigned int i = 0; i < threads.size(); i++) {
		threads[i].join();
		if (success) {
			success = data[i]->success;
			verticies.insert(verticies.end(), data[i]->ret.begin(), data[i]->ret.end());
			data[i]->ret.clear();
		}
	}

	if (success) {
		float gzmin = FLT_MAX, gzmax = -FLT_MAX;
		for (unsigned int i = 0; i < threads.size(); i++) {
			if (data[i]->zmin < gzmin) gzmin = data[i]->zmin;
			if (data[i]->zmax > gzmax) gzmax = data[i]->zmax;
		}
		zmin = gzmin;
		zmax = gzmax;

		normalize(s);
		generateIndiciesAndNormals(s);
	}

	for (gendata* g : data)
		delete g;
}
