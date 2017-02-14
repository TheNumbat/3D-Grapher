
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

struct spr_domain {
	float tmin, tmax, pmin, pmax, rmin, rmax;
	int trez, prez;
};

struct settings {
	bool wireframe;
	bool lighting;
	bool axisnormalization;
	float graphopacity;
	float ambientLighting;
	float fov;
	rect_domain rdom;
	cyl_domain cdom;
	spr_domain sdom;
	cam_type camtype;
};