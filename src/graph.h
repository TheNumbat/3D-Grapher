
#pragma once

#include <thread>
#include <limits>

struct gendata {
	gendata() {
		zmin = FLT_MAX;
		zmax = -FLT_MAX;
	};
	state* s;
	vector<float> ret;
	float zmin, zmax, xmin, dx, dy;
	unsigned int txrez;
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