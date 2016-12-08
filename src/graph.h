
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
	0.0f, 0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,  	1.0f, 0.0f, 0.0f,

	0.0f, 0.0f, 0.0f,  	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f,  	0.0f, 1.0f, 0.0f,

	0.0f, 0.0f, 0.0f,  	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f,  	0.0f, 0.0f, 1.0f
};

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

	s->g.verticies.clear();

	float dx = (s->g.xmax - s->g.xmin) / s->g.xrez;
	float dy = (s->g.ymax - s->g.ymin) / s->g.yrez;
	float xmin = s->g.xmin;
	unsigned int txDelta = s->g.xrez / numthreads;
	unsigned int txLast = s->g.xrez - (numthreads - 1) * txDelta + 1;

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
			threads.push_back(thread(genthread, data.back()));

			xmin += txDelta * dx;
		}
	}
	for (unsigned int i = 0; i < threads.size(); i++) {
		threads[i].join();
		s->g.verticies.insert(s->g.verticies.end(), data[i]->ret.begin(), data[i]->ret.end());
		data[i]->ret.clear();
	}

	for (unsigned int x = 0; x < s->g.xrez; x++) {
		for (unsigned int y = 0; y < s->g.yrez; y++) {
			GLuint index = x * (s->g.yrez + 1) + y;

			if (!isnan(s->g.verticies[index * 3 + 2]) &&
				!isinf(s->g.verticies[index * 3 + 2])) {
				s->g.indicies.push_back(index);
				s->g.indicies.push_back(index + 1);
				s->g.indicies.push_back(index + s->g.yrez + 1);

				s->g.indicies.push_back(index + 1);
				s->g.indicies.push_back(index + s->g.yrez + 1);
				s->g.indicies.push_back(index + s->g.yrez + 2);
			}
		}
	}

	axes[x_min] = s->g.xmin;
	axes[x_max] = s->g.xmax;
	axes[y_min] = s->g.ymin;
	axes[y_max] = s->g.ymax;

	float zmin = FLT_MAX, zmax = -FLT_MAX;
	for (unsigned int i = 0; i < threads.size(); i++) {
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

void regengraph(state* s) {
	
	vector<op> new_eq;

	if (!in(s->g.eq_str, new_eq)) {
		return;
	}
	s->g.indicies.clear();
	s->g.verticies.clear();
	s->g.eq = new_eq;

	printeq(cout, s->g.eq);

	Uint64 start = SDL_GetPerformanceCounter();
	gengraph(s);
	Uint64 end = SDL_GetPerformanceCounter();
	cout << "time: " << (float)(end - start) / SDL_GetPerformanceFrequency() << endl;

	glBindVertexArray(s->graphVAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, s->graphVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->EBO);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * s->g.indicies.size(), s->g.indicies.size() ? &s->g.indicies[0] : NULL, GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * s->g.verticies.size(), s->g.verticies.size() ? &s->g.verticies[0] : NULL, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
	}

	glBindVertexArray(s->axisVAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, s->axisVBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
	}
}