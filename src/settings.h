
#pragma once

#include "graph.h"
#include "cam.h"

struct rect_domain {
	float xmin, xmax, ymin, ymax, zmin, zmax;
	int xrez, yrez;
};

struct cyl_domain {
	float zmin, zmax, tmin, tmax, rmin, rmax;
	int zrez, trez;
};

struct settings {
	bool wireframe;
	bool lighting;
	bool axisnormalization;
	float graphopacity;
	rect_domain rdom;
	cyl_domain cdom;
	dimension display;
	cam_type camtype;
};