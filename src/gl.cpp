
#include "gl.h"

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
PFNGLUNIFORM3FPROC					glUniform3f;
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

texture::texture() {
	glGenTextures(1, &tex);
}

texture::~texture() {
	glDeleteTextures(1, &tex);
}

void texture::load(SDL_Surface* surf) {
	use();
	SDL_Surface* temp = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, temp->w, temp->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, temp->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SDL_FreeSurface(temp);
}

void texture::use() {
	glBindTexture(GL_TEXTURE_2D, tex);
}

shader::~shader() {
	glDeleteShader(program);
}

void shader::load(const GLchar* vertex, const GLchar* fragment) {
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

void shader::use() {
	glUseProgram(program);
}

GLuint shader::getUniform(const GLchar* name) {
	return glGetUniformLocation(program, name);
}

textured_rect::textured_rect() {
	x = y = w = h = 0;
	needsupdate = true;
}

textured_rect::~textured_rect() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

textured_rect::textured_rect(float _x, float _y, float _w, float _h) {
	x = _x; y = _y; w = _w; h = _h;
	needsupdate = true;
}

void textured_rect::gen() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
}

void textured_rect::set(float _x, float _y, float _w, float _h) {
	x = _x; y = _y; w = _w; h = _h;
	needsupdate = true;
}

void textured_rect::update_bounds(int wW, int wH) {
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

void textured_rect::render(int wW, int wH, shader& s) {
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
	glUniform3f					= (PFNGLUNIFORM3FPROC)					wglGetProcAddress("glUniform3f");
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

const GLchar* graph_vertex = {
	"#version 330 core\n"

	"layout (location = 0) in vec3 position;\n"

	"uniform mat4 modelviewproj;\n"

	"void main() {\n"
	"	gl_Position = modelviewproj * vec4(position, 1.0f);\n"
	"}\n"
};

const GLchar* graph_fragment = {
	"#version 330 core\n"

	"uniform vec4 vcolor;\n"
	"out vec4 color;\n"

	"void main() {\n"
	"	color = vcolor;\n"
	"}\n"
};

const GLchar* graph_vertex_lighting = {
	"#version 330 core\n"

	"layout (location = 0) in vec3 position;\n"
	"layout (location = 1) in vec3 normal;\n"

	"uniform mat4 modelviewproj;\n"
	"uniform mat4 model;\n"
	"out vec3 norm;\n"
	"out vec3 fragPos;\n"

	"void main() {\n"
	"	gl_Position = modelviewproj * vec4(position, 1.0f);\n"
	"	norm = vec3(model * vec4(normal, 1.0f));;\n"
	"   fragPos = vec3(model * vec4(position, 1.0f));\n"
	"}\n"
};

const GLchar* graph_fragment_lighting = {
	"#version 330 core\n"

	"uniform vec4 vcolor;\n"
	"uniform vec3 lightColor;\n"
	"uniform vec3 lightPos;\n"
	"in vec3 norm;\n"
	"in vec3 fragPos;\n"
	"out vec4 color;\n"

	"void main() {\n"
	"   float ambientStrength = 1.0f;\n"
	"	vec3 ambient = ambientStrength * lightColor;\n"
	"   vec3 lightDir = normalize(lightPos - fragPos);\n"
	"   float diff = max(abs(dot(norm, lightDir)), 0.0f);\n"
	"   vec3 diffuse = diff * lightColor;\n"
	"	color = vec4(ambient, 1.0f) * vec4(diffuse, 1.0f) * vcolor;\n"
	"}\n"
};

const GLchar* ui_vertex = {
	"#version 330 core\n"

	"layout (location = 0) in vec2 position;\n"
	"layout (location = 1) in vec2 tcoord;\n"

	"out vec2 coord;\n"

	"void main() {\n"
	"	gl_Position = vec4(position, 0.0f, 1.0f);\n"
	"	coord = tcoord;\n"
	"}\n"
};

const GLchar* ui_fragment = {
	"#version 330 core\n"

	"in vec2 coord;\n"
	"out vec4 color;\n"
	"uniform sampler2D tex;\n"

	"void main() {\n"
	"	color = texture(tex, vec2(coord.x, 1.0f - coord.y));\n"
	"}\n"
};

const GLchar* rect_vertex = {
	"#version 330 core\n"

	"layout (location = 0) in vec2 position;\n"

	"uniform vec4 vcolor;\n"
	"out vec4 fcolor;\n"

	"void main() {\n"
	"	gl_Position = vec4(position, 0.0f, 1.0f);\n"
	"	fcolor = vcolor;\n"
	"}\n"
};

const GLchar* rect_fragment = {
	"#version 330 core\n"

	"in vec4 fcolor;\n"
	"out vec4 color;\n"

	"void main() {\n"
	"	color = fcolor;\n"
	"}\n"
};

const GLchar* axis_vertex = {
	"#version 330 core\n"

	"layout (location = 0) in vec3 position;\n"
	"layout (location = 1) in vec3 color;\n"

	"out vec3 vcolor;\n"

	"uniform mat4 modelviewproj;\n"

	"void main() {\n"
	"	gl_Position = modelviewproj * vec4(position, 1.0f);\n"
	"	vcolor = color;\n"
	"}\n"
};

const GLchar* axis_fragment = {
	"#version 330 core\n"

	"in vec3 vcolor;\n"
	"out vec4 color;\n"

	"void main() {\n"
	"	color = vec4(vcolor, 1.0f);\n"
	"}\n"
};