
#pragma once

extern const GLchar* graph_vertex;
extern const GLchar* graph_fragment;
extern const GLchar* graph_vertex_lighting;
extern const GLchar* graph_fragment_lighting;
extern const GLchar* graph_vertex_norm;
extern const GLchar* graph_fragment_norm;
extern const GLchar* axis_vertex;
extern const GLchar* axis_fragment;

struct shader {
	~shader();
	void load(const GLchar* vertex, const GLchar* fragment);
	void use();
	GLuint getUniform(const GLchar* name);
	GLuint program;
};

