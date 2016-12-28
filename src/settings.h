
#pragma once

#include "graph.h"
#include "cam.h"

struct settings {
	bool wireframe;
	bool lighting;
	bool axisnormalization;
	float graphopacity;
	int antialiasing;
	dimension display;
	cam_type camtype;
};