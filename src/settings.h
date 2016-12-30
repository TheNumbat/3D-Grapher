
#pragma once

#include "graph.h"
#include "cam.h"

struct settings {
	bool wireframe;
	bool lighting;
	bool axisnormalization;
	bool antialiasing;
	float graphopacity;
	float xmin, xmax, ymin, ymax, zmin, zmax;
	int xrez, yrez;
	dimension display;
	cam_type camtype;
};