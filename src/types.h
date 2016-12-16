
#pragma once

enum inputstate {
	in_idle,
	in_cam,
	in_text
};

#include <vector>
#include <functional>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_opengl.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "gl.h"

using namespace glm;
using namespace std;

typedef int op;

struct cam {
	vec3 pos, front, up, right, globalUp;
	float pitch, yaw, speed, fov;
	Uint32 lastUpdate;
};

struct graph {
	graph(int id, string s, float xmi, float xma, float ymi, float yma, float xr, float yr) : eq_str(s) {
		ID = id;
		xmin = xmi; xmax = xma; ymin = ymi; ymax = yma;
		xrez = xr; yrez = yr;
	}
	void gen() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
	}
	~graph() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}
	void send() {
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verticies.size(), verticies.size() ? &verticies[0] : NULL, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indicies.size(), indicies.size() ? &indicies[0] : NULL, GL_STATIC_DRAW);

		glBindVertexArray(0);
	}
	vector<op> eq;
	string eq_str;
	vector<GLfloat> verticies;
	vector<GLuint> indicies;
	GLuint VAO, VBO, EBO;
	float xmin, xmax, ymin, ymax;
	unsigned int xrez, yrez;
	int ID;
};

struct state;
struct gendata {
	gendata() {
		zmin = FLT_MAX;
		zmax = -FLT_MAX;
	};
	state* s;
	vector<float> ret;
	float zmin, zmax, xmin, dx, dy;
	unsigned int txrez;
};

struct widget {
	virtual int render(int w, int h, int ui_w, int x, int y, shader& program) = 0;
	virtual bool process(SDL_Event ev, int w, state* s) = 0;
	point pts[6];
	int current_y, current_yh;
	bool active, should_remove;
};

struct UI;
struct state {
	SDL_Window* window;
	int w, h;
	SDL_GLContext context;
	GLuint axisVAO, axisVBO;
	shader graph_s, axis_s, UI_s, rect_s;

	vector<graph*> graphs;
	cam c;
	UI* ui;
	inputstate instate;

	bool running;
};

#include "ui.h"