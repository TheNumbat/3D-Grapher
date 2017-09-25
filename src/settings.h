
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

struct graph_settings {
	bool wireframe         = false;
	bool lighting          = true;
	bool axisnormalization = false;
	float opacity     	   = 1.0f;
	float ambientLighting  = 0.0f;
	union {
		rect_domain rdom;       // { -10, 10, -10, 10, -10, 10, 200, 200 };
		cyl_domain cdom;        // { 0, 1, 0, 2 * val_pi, 0, 10, 200, 200 };
		spr_domain sdom;        // { 0, 2 * val_pi, 0, val_pi, 0, 10, 200, 200 };
	};
};

struct camera_settings {
	cam_type camtype       = cam_3d_static;
	float fov              = 60.0f;
};
