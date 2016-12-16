
#pragma once

#include <thread>
#include <limits>
#include "exp.h"

const int x_min = 0;
const int x_max = 6;
const int y_min = 13;
const int y_max = 19;
const int z_min = 26;
const int z_max = 32;

GLfloat axes[] = {
   -10.0f,  0.0f ,  0.0f ,		1.0f, 0.0f, 0.0f,
	10.0f,  0.0f ,  0.0f ,		1.0f, 0.0f, 0.0f,

	0.0f , -10.0f,  0.0f ,		0.0f, 1.0f, 0.0f,
	0.0f ,  10.0f,  0.0f ,		0.0f, 1.0f, 0.0f,

	0.0f ,  0.0f , -10.0f,		0.0f, 0.0f, 1.0f,
	0.0f ,  0.0f ,  10.0f,		0.0f, 0.0f, 1.0f
};

void sendAxes(state* s) {
	float xmin = FLT_MAX, xmax = -FLT_MAX;
	float ymin = FLT_MAX, ymax = -FLT_MAX;
	float zmin = FLT_MAX, zmax = -FLT_MAX;
	
	if (s->graphs.size()) {
		for (graph* g : s->graphs) {
			if (g->xmin < xmin) xmin = g->xmin;
			if (g->xmax > xmax) xmax = g->xmax;
			if (g->ymin < ymin) ymin = g->ymin;
			if (g->ymax > ymax) ymax = g->ymax;
			if (g->zmin < zmin) zmin = g->zmin;
			if (g->zmax > zmax) zmax = g->zmax;
		}

		if (xmin > 0) xmin = 0;
		if (xmax < 0) xmax = 0;
		if (ymin > 0) ymin = 0;
		if (ymax < 0) ymax = 0;
		if (zmin > 0) zmin = 0;
		if (zmax < 0) zmax = 0;
	}
	else {
		xmin = ymin = zmin = -10;
		xmax = ymax = zmax = 10;
	}

	axes[x_min] = xmin;
	axes[y_min] = ymin;
	axes[z_min] = zmin;
	axes[x_max] = xmax;
	axes[y_max] = ymax;
	axes[z_max] = zmax;

	glBindVertexArray(s->axisVAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, s->axisVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);
	}
}

float clamp(float one, float two) {
	if (one > 0)
		return one - fmod(one, two);
	else if (one < 0)
		return one + fmod(-one, two);
	else
		return one;
}

void genthread(gendata* g, int index) {
	float x = g->xmin;
	for (unsigned int tx = 0; tx < g->txrez; tx++, x += g->dx) {
		float y = g->s->graphs[index]->ymin;
		for (unsigned int ty = 0; ty <= g->s->graphs[index]->yrez; ty++, y += g->dy) {
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

	float dx = (s->graphs[index]->xmax - s->graphs[index]->xmin) / s->graphs[index]->xrez;
	float dy = (s->graphs[index]->ymax - s->graphs[index]->ymin) / s->graphs[index]->yrez;
	float xmin = s->graphs[index]->xmin;
	unsigned int txDelta = s->graphs[index]->xrez / numthreads;
	unsigned int txLast = s->graphs[index]->xrez - (numthreads - 1) * txDelta + 1;

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

	for (unsigned int x = 0; x < s->graphs[index]->xrez; x++) {
		for (unsigned int y = 0; y < s->graphs[index]->yrez; y++) {
			GLuint i_index = x * (s->graphs[index]->yrez + 1) + y;

			if (!isnan(s->graphs[index]->verticies[i_index * 3 + 2]) &&
				!isinf(s->graphs[index]->verticies[i_index * 3 + 2])) {
				s->graphs[index]->indicies.push_back(i_index);
				s->graphs[index]->indicies.push_back(i_index + 1);
				s->graphs[index]->indicies.push_back(i_index + s->graphs[index]->yrez + 1);

				s->graphs[index]->indicies.push_back(i_index + 1);
				s->graphs[index]->indicies.push_back(i_index + s->graphs[index]->yrez + 1);
				s->graphs[index]->indicies.push_back(i_index + s->graphs[index]->yrez + 2);
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