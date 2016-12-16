

#pragma once

#include <SDL_opengl.h>
#include <string>
#include <SDL.h>
#include "shaders.h"

using namespace std;

PFNGLBINDVERTEXARRAYPROC			glBindVertexArray;
PFNGLDELETEBUFFERSPROC				glDeleteBuffers;
PFNGLBINDBUFFERPROC					glBindBuffer;
PFNGLBUFFERDATAPROC					glBufferData;
PFNGLUSEPROGRAMPROC					glUseProgram;
PFNGLENABLEVERTEXATTRIBARRAYPROC	glEnableVertexAttribArray;
PFNGLVERTEXATTRIBPOINTERPROC		glVertexAttribPointer;
PFNGLUNIFORMMATRIX4FVPROC			glUniformMatrix4fv;
PFNGLGETUNIFORMLOCATIONPROC			glGetUniformLocation;
PFNGLUNIFORM4FPROC					glUniform4f;
PFNGLCREATESHADERPROC				glCreateShader;
PFNGLSHADERSOURCEPROC				glShaderSource;
PFNGLCOMPILESHADERPROC				glCompileShader;
PFNGLCREATEPROGRAMPROC				glCreateProgram;
PFNGLDELETESHADERPROC				glDeleteShader;
PFNGLATTACHSHADERPROC				glAttachShader;
PFNGLLINKPROGRAMPROC				glLinkProgram;
PFNGLGENVERTEXARRAYSPROC			glGenVertexArrays;
PFNGLGENBUFFERSPROC					glGenBuffers;
PFNGLDELETEVERTEXARRAYSPROC			glDeleteVertexArrays;
PFNGLDELETEPROGRAMPROC				glDeleteProgram;
PFNGLACTIVETEXTUREPROC				_glActiveTexture; 
PFNGLUNIFORM1IPROC					glUniform1i;
PFNGLDISABLEVERTEXATTRIBARRAYPROC	glDisableVertexAttribArray;

struct point {
	float x, y, tx, ty;
};

struct texture {
	texture() {
		glGenTextures(1, &tex);
	}
	~texture() {
		glDeleteTextures(1, &tex);
	}
	void load(SDL_Surface* surf) {
		use();
		SDL_Surface* temp = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, temp->w, temp->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, temp->pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		SDL_FreeSurface(temp);
	}
	void use() {
		glBindTexture(GL_TEXTURE_2D, tex);
	}
	GLuint tex;
};

struct shader {
	~shader() {
		glDeleteShader(program);
	}
	void load(const GLchar* vertex, const GLchar* fragment) {
		GLuint v, f;
		v = glCreateShader(GL_VERTEX_SHADER);
		f = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(v, 1, &vertex, NULL);
		glShaderSource(f, 1, &fragment, NULL);
		glCompileShader(v);
		glCompileShader(f);
		program = glCreateProgram();
		glAttachShader(program, v);
		glAttachShader(program, f);
		glLinkProgram(program);
		glDeleteShader(v);
		glDeleteShader(f);
	}
	void use() {
		glUseProgram(program);
	}
	GLuint getUniform(const GLchar* name) {
		return glGetUniformLocation(program, name);
	}
	GLuint program;
};

struct textured_rect {
	textured_rect() {
		x = y = w = h = 0;
		needsupdate = true;
	}
	~textured_rect() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}
	textured_rect(float _x, float _y, float _w, float _h) {
		x = _x; y = _y; w = _w; h = _h;
		needsupdate = true;
	}
	void gen() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
	}
	void set(float _x, float _y, float _w, float _h) {
		x = _x; y = _y; w = _w; h = _h;
		needsupdate = true;
	}
	void update_bounds(int wW, int wH) {
		gl_points[0] = { -1.0f + 2.0f * x / wW                , 1.0f - 2.0f * y / wH      , 0.0f, 1.0f };
		gl_points[1] = { -1.0f + 2.0f * x / wW                , 1.0f - 2.0f * (y + h) / wH, 0.0f, 0.0f };
		gl_points[2] = { -1.0f + 2.0f * x / wW + 2.0f * w / wW, 1.0f - 2.0f * y / wH      , 1.0f, 1.0f };

		gl_points[3] = { -1.0f + 2.0f * x / wW                , 1.0f - 2.0f * (y + h) / wH, 0.0f, 0.0f };
		gl_points[4] = { -1.0f + 2.0f * x / wW + 2.0f * w / wW, 1.0f - 2.0f *  y / wH     , 1.0f, 1.0f };
		gl_points[5] = { -1.0f + 2.0f * x / wW + 2.0f * w / wW, 1.0f - 2.0f * (y + h) / wH, 1.0f, 0.0f };
		
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(gl_points), gl_points, GL_STATIC_DRAW);
		glBindVertexArray(0);

		needsupdate = false;
	}
	void render(int wW, int wH, shader& s) {
		if (needsupdate)
			update_bounds(wW, wH);
		glBindVertexArray(VAO);
		s.use();
		tex.use();

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		glBindVertexArray(0);
	}
	bool needsupdate;
	float x, y, w, h;
	point gl_points[6];
	texture tex;
	GLuint VAO, VBO;
};

void setupFuns() {
	glBindVertexArray			= (PFNGLBINDVERTEXARRAYPROC)			wglGetProcAddress("glBindVertexArray");
	glDeleteBuffers				= (PFNGLDELETEBUFFERSPROC)				wglGetProcAddress("glDeleteBuffers");
	glBindBuffer				= (PFNGLBINDBUFFERPROC)					wglGetProcAddress("glBindBuffer");
	glBufferData				= (PFNGLBUFFERDATAPROC)					wglGetProcAddress("glBufferData");
	glUseProgram				= (PFNGLUSEPROGRAMPROC)					wglGetProcAddress("glUseProgram");
	glEnableVertexAttribArray	= (PFNGLENABLEVERTEXATTRIBARRAYPROC)	wglGetProcAddress("glEnableVertexAttribArray");
	glVertexAttribPointer		= (PFNGLVERTEXATTRIBPOINTERPROC)		wglGetProcAddress("glVertexAttribPointer");
	glUniformMatrix4fv			= (PFNGLUNIFORMMATRIX4FVPROC)			wglGetProcAddress("glUniformMatrix4fv");
	glGetUniformLocation		= (PFNGLGETUNIFORMLOCATIONPROC)			wglGetProcAddress("glGetUniformLocation");
	glUniform4f					= (PFNGLUNIFORM4FPROC)					wglGetProcAddress("glUniform4f");
	glCreateShader				= (PFNGLCREATESHADERPROC)				wglGetProcAddress("glCreateShader");
	glShaderSource				= (PFNGLSHADERSOURCEPROC)				wglGetProcAddress("glShaderSource");
	glCompileShader				= (PFNGLCOMPILESHADERPROC)				wglGetProcAddress("glCompileShader");
	glCreateProgram				= (PFNGLCREATEPROGRAMPROC)				wglGetProcAddress("glCreateProgram");
	glDeleteShader				= (PFNGLDELETESHADERPROC)				wglGetProcAddress("glDeleteShader");
	glAttachShader				= (PFNGLATTACHSHADERPROC)				wglGetProcAddress("glAttachShader");
	glLinkProgram				= (PFNGLLINKPROGRAMPROC)				wglGetProcAddress("glLinkProgram");
	glGenVertexArrays			= (PFNGLGENVERTEXARRAYSPROC)			wglGetProcAddress("glGenVertexArrays");
	glGenBuffers				= (PFNGLGENBUFFERSPROC)					wglGetProcAddress("glGenBuffers");
	glDeleteVertexArrays		= (PFNGLDELETEVERTEXARRAYSPROC)			wglGetProcAddress("glDeleteVertexArrays");
	glDeleteProgram				= (PFNGLDELETEPROGRAMPROC)				wglGetProcAddress("glDeleteProgram");
	_glActiveTexture			= (PFNGLACTIVETEXTUREPROC)				wglGetProcAddress("glActiveTexture");
	glUniform1i					= (PFNGLUNIFORM1IPROC)					wglGetProcAddress("glUniform1i");
	glDisableVertexAttribArray  = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)   wglGetProcAddress("glDisableVertexAttribArray");
}
