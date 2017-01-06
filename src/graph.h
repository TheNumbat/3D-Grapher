
#pragma once

#include <vector>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "gl.h"
#include "exp.h"

using namespace std;
using namespace glm;

struct state;

enum dimension {
	dim_2d,
	dim_3d,
	dim_4d
};

enum graph_type {
	graph_func,
	graph_cylindrical,
	graph_spherical,
	graph_partial,
	graph_integral,
	graph_surface,
	graph_para_curve,
	graph_para_surface,
	graph_vec_feild
};

const int x_min = 0;
const int x_max = 6;
const int y_min = 13;
const int y_max = 19;
const int z_min = 26;
const int z_max = 32;

extern GLfloat axes[];

struct graph {
	graph(int id, string s = " ");
	virtual void generate(state* s) = 0;
	virtual ~graph();
	void draw(state* s, mat4 model, mat4 view, mat4 proj);
	void gen();
	void send();
	void normalize(state* s);
	vector<GLfloat> verticies;
	vector<GLuint> indicies;
	vector<vec3> normals;
	GLuint VAO, VBO, EBO, normVBO;
	dimension dim;
	graph_type type;
	vector<op> eq;
	string eq_str;
	float zmin, zmax;
	int ID;
};

struct cyl_graph : public graph {
	struct gendata {
		gendata() {
			rmin = FLT_MAX;
			rmax = -FLT_MAX;
		};
		state* s;
		vector<float> ret;
		float rmin, rmax, zmin, dz, dt;
		int tzrez, ID;
	};

	cyl_graph(int id, string s = " ");
	void generate(state* s);
	static void genthread(gendata* g);
};

struct fxy_graph : public graph {
	struct gendata {
		gendata() {
			zmin = FLT_MAX;
			zmax = -FLT_MAX;
		};
		state* s;
		vector<float> ret;
		float zmin, zmax, xmin, dx, dy;
		int txrez, ID;
	};

	fxy_graph(int id, string s = " ");
	void generate(state* s);
	static void genthread(gendata* g);
};

void updateAxes(state* s);
void resetCam(state* s);
void regengraph(state* s, int index);
void regenall(state* s);
int getIndex(state* s, int ID);