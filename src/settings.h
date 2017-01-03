
#pragma once

#include "graph.h"
#include "cam.h"

struct rect_domain {
	float xmin, xmax, ymin, ymax, zmin, zmax;
	int xrez, yrez;
};

struct cyl_domain {
	float rmin, rmax, tmin, tmax, zmin, zmax;
	int rrez, trez;
};

struct settings {
	bool wireframe;
	bool lighting;
	bool axisnormalization;
	bool antialiasing;
	float graphopacity;
	rect_domain rdom;
	cyl_domain cdom;
	dimension display;
	cam_type camtype;
};