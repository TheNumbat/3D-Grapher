
#pragma once

#include "graph.h"
#include "cam.h"

struct settings {
	bool wireframe;
	bool lighting;
	bool axisnormalization;
	float graphopacity;
	int antialiasing;
	int xmin, xmax, ymin, ymax, xrez, yrez;
	dimension display;
	cam_type camtype;
};