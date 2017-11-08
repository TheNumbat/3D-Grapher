
#pragma once

struct rect_domain {
	float xmin, xmax, ymin, ymax;
	int xrez, yrez;
};

struct cyl_domain {
	float zmin, zmax, tmin, tmax;
	int zrez, trez;
};

struct spr_domain {
	float tmin, tmax, pmin, pmax;
	int trez, prez;
};

struct para_domain {
	float tmin, tmax;
	int trez;
};

enum class calculus {
	none = 0,
	part_x,
	part_y,
	part2_x,
	part2_y,
	part3_x,
	part3_y,
};

struct graph_settings {
	bool wireframe         = false;
	bool lighting          = true;
	bool axisnormalization = false;
	bool normal_colors	   = false;
	float opacity     	   = 1.0f;
	float ambientLighting  = 0.0f;
	union {
		rect_domain rdom;       // { -10, 10, -10, 10, -10, 10, 200, 200 };
		cyl_domain cdom;        // { 0, 1, 0, 2 * val_pi, 0, 10, 200, 200 };
		spr_domain sdom;        // { 0, 2 * val_pi, 0, val_pi, 0, 10, 200, 200 };
		para_domain pdom;		// { 0, 10, 100 }
	};
	calculus calc = calculus::none;
};
