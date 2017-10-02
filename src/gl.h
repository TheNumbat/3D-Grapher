
#pragma once

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
extern PFNGLUNIFORM1FPROC					glUniform1f;
extern PFNGLBINDSAMPLERPROC					glBindSampler;
extern PFNGLBLENDEQUATIONSEPARATEPROC		glBlendEquationSeparate;
extern PFNGLBLENDFUNCSEPARATEPROC			glBlendFuncSeparate;
extern PFNGLGETATTRIBLOCATIONPROC			glGetAttribLocation;
extern PFNGLDETACHSHADERPROC				glDetachShader;
extern PFNGLBLENDEQUATIONPROC				_glBlendEquation;

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

void setupFuns();
