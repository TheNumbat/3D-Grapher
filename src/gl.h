
#pragma once

extern const GLchar* graph_vertex;
extern const GLchar* graph_fragment;
extern const GLchar* axis_vertex;
extern const GLchar* axis_fragment;

struct shader {
	~shader();
	void load(const GLchar* vertex, const GLchar* fragment);
	void use();
	GLuint getUniform(const GLchar* name);
	GLuint program;
};

void debug_proc(GLenum glsource, GLenum gltype, GLuint id, GLenum severity, GLsizei length, const GLchar* glmessage, const void* up);
