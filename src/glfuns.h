
#pragma once

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
}