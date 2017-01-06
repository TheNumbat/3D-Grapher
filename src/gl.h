

#pragma once

#include <SDL2/SDL_opengl.h>
#include <string>
#include <SDL2/SDL.h>

using namespace std;

extern PFNGLBINDVERTEXARRAYPROC				glBindVertexArray;
extern PFNGLDELETEBUFFERSPROC				glDeleteBuffers;
extern PFNGLBINDBUFFERPROC					glBindBuffer;
extern PFNGLBUFFERDATAPROC					glBufferData;
extern PFNGLUSEPROGRAMPROC					glUseProgram;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC		glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC			glVertexAttribPointer;
extern PFNGLUNIFORMMATRIX4FVPROC			glUniformMatrix4fv;
extern PFNGLGETUNIFORMLOCATIONPROC			glGetUniformLocation;
extern PFNGLUNIFORM4FPROC					glUniform4f;
extern PFNGLUNIFORM3FPROC					glUniform3f;
extern PFNGLCREATESHADERPROC				glCreateShader;
extern PFNGLSHADERSOURCEPROC				glShaderSource;
extern PFNGLCOMPILESHADERPROC				glCompileShader;
extern PFNGLCREATEPROGRAMPROC				glCreateProgram;
extern PFNGLDELETESHADERPROC				glDeleteShader;
extern PFNGLATTACHSHADERPROC				glAttachShader;
extern PFNGLLINKPROGRAMPROC					glLinkProgram;
extern PFNGLGENVERTEXARRAYSPROC				glGenVertexArrays;
extern PFNGLGENBUFFERSPROC					glGenBuffers;
extern PFNGLDELETEVERTEXARRAYSPROC			glDeleteVertexArrays;
extern PFNGLDELETEPROGRAMPROC				glDeleteProgram;
extern PFNGLACTIVETEXTUREPROC				_glActiveTexture; 
extern PFNGLUNIFORM1IPROC					glUniform1i;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC	glDisableVertexAttribArray;

extern const GLchar* graph_vertex;
extern const GLchar* graph_fragment;
extern const GLchar* graph_vertex_lighting;
extern const GLchar* graph_fragment_lighting;
extern const GLchar* ui_vertex;
extern const GLchar* ui_fragment;
extern const GLchar* rect_vertex;
extern const GLchar* rect_fragment;
extern const GLchar* axis_vertex;
extern const GLchar* axis_fragment;

struct point {
	float x, y, tx, ty;
};

struct texture {
	texture();
	~texture();
	void load(SDL_Surface* surf);
	void use();
	GLuint tex;
};

struct shader {
	~shader();
	void load(const GLchar* vertex, const GLchar* fragment);
	void use();
	GLuint getUniform(const GLchar* name);
	GLuint program;
};

struct textured_rect {
	textured_rect();
	textured_rect(float _x, float _y, float _w, float _h);
	~textured_rect();
	void gen();
	void set(float _x, float _y, float _w, float _h);
	void update_bounds(int wW, int wH);
	void render(int wW, int wH, shader& s);
	bool needsupdate;
	float x, y, w, h;
	point gl_points[6];
	texture tex;
	GLuint VAO, VBO;
};

void setupFuns();