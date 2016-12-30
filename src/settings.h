
#pragma once

#include "graph.h"
#include "cam.h"

struct settings {
	bool wireframe;
	bool lighting;
	bool axisnormalization;
	bool antialiasing;
	float graphopacity;
	int xmin, xmax, ymin, ymax, xrez, yrez;
	dimension display;
	cam_type camtype;
};