
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

enum graph_type {
	graph_func,				// done
	graph_cylindrical,		// done
	graph_spherical,		// done
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
	virtual ~graph();

	virtual void normalize(state* s);
	virtual void generate(state* s) = 0;
	virtual void generateIndiciesAndNormals(state* s);
	virtual void draw(state* s, mat4 model, mat4 view, mat4 proj);

	void gen();
	void send();

	vector<GLfloat> verticies;
	vector<GLuint> indicies;
	vector<vec3> normals;

	int ID;
	string eq_str;
	vector<op> eq;
	graph_type type;
	float zmin, zmax;
	GLuint VAO, VBO, EBO, normVBO;
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
		bool success;
	};

	fxy_graph(int id, string s = " ");
	void generate(state* s);
	static void genthread(gendata* g);
};

struct cyl_graph : public graph {
	struct gendata {
		gendata() {
			gzmin = FLT_MAX;
			gzmax = -FLT_MAX;
		};
		state* s;
		vector<float> ret;
		float gzmin, gzmax, zmin, dz, dt;
		int tzrez, ID;
		bool success;
	};

	cyl_graph(int id, string s = " ");
	void generate(state* s);
	static void genthread(gendata* g);
};

struct spr_graph : public graph {
	struct gendata {
		gendata() {
			zmin = FLT_MAX;
			zmax = -FLT_MAX;
		};
		state* s;
		vector<float> ret;
		float zmin, zmax, pmin, dt, dp;
		int tprez, ID;
		bool success;
	};

	spr_graph(int id, string s = " ");
	void generate(state* s);
	static void genthread(gendata* g);
};

void updateAxes(state* s);
void resetCam(state* s);
void regengraph(state* s, int index);
void regenall(state* s);
int getIndex(state* s, int ID);