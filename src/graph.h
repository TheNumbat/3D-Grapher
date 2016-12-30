
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
	graph(int id, string s, float xmi, float xma, float ymi, float yma, unsigned int xr, unsigned int yr);
	~graph();
	void gen();
	void send();
	void draw(state* s, mat4& modelveiwproj);
	
	vector<op> eq;
	string eq_str;

	vector<GLfloat> verticies;
	vector<GLuint> indicies;
	GLuint VAO, VBO, EBO;

	float zmin, zmax;
	int ID;

	dimension dim;
	graph_type type;
};

void updateAxes(state* s);
void regengraph(state* s, int index);
void regenall(state* s);
int getIndex(state* s, int ID);