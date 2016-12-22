
#pragma once

#include <vector>
#include "gl.h"
#include "exp.h"

using namespace std;

struct state;

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
	vector<op> eq;
	string eq_str;
	vector<GLfloat> verticies;
	vector<GLuint> indicies;
	GLuint VAO, VBO, EBO;
	float xmin, xmax, ymin, ymax, zmin, zmax;
	unsigned int xrez, yrez;
	int ID;
};

void sendAxes(state* s);
void regengraph(state* s, int index);
int getIndex(state* s, int ID);