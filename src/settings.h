
#pragma once

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
	bool wireframe         = false;
	bool lighting          = true;
	bool axisnormalization = false;
	float graphopacity     = 1.0f;
	float ambientLighting  = 0.0f;
	float fov              = 60.0f;
	rect_domain rdom       = { -10, 10, -10, 10, -10, 10, 200, 200 };
	cyl_domain cdom        = { 0, 1, 0, 2 * val_pi, 0, 10, 200, 200 };
	spr_domain sdom        = { 0, 2 * val_pi, 0, val_pi, 0, 10, 200, 200 };
	cam_type camtype       = cam_3d_static;
};