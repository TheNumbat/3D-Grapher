
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
PFNGLUNIFORM1FPROC					glUniform1f;
PFNGLBINDSAMPLERPROC				glBindSampler;
PFNGLBLENDEQUATIONSEPARATEPROC		glBlendEquationSeparate;
PFNGLBLENDFUNCSEPARATEPROC			glBlendFuncSeparate;
PFNGLGETATTRIBLOCATIONPROC			glGetAttribLocation;
PFNGLDETACHSHADERPROC				glDetachShader;
PFNGLBLENDEQUATIONPROC				_glBlendEquation;

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

void setupFuns() {
	glBindVertexArray			= (PFNGLBINDVERTEXARRAYPROC)			SDL_GL_GetProcAddress("glBindVertexArray");
	glDeleteBuffers				= (PFNGLDELETEBUFFERSPROC)				SDL_GL_GetProcAddress("glDeleteBuffers");
	glBindBuffer				= (PFNGLBINDBUFFERPROC)					SDL_GL_GetProcAddress("glBindBuffer");
	glBufferData				= (PFNGLBUFFERDATAPROC)					SDL_GL_GetProcAddress("glBufferData");
	glUseProgram				= (PFNGLUSEPROGRAMPROC)					SDL_GL_GetProcAddress("glUseProgram");
	glEnableVertexAttribArray	= (PFNGLENABLEVERTEXATTRIBARRAYPROC)	SDL_GL_GetProcAddress("glEnableVertexAttribArray");
	glVertexAttribPointer		= (PFNGLVERTEXATTRIBPOINTERPROC)		SDL_GL_GetProcAddress("glVertexAttribPointer");
	glUniformMatrix4fv			= (PFNGLUNIFORMMATRIX4FVPROC) 			SDL_GL_GetProcAddress("glUniformMatrix4fv");
	glGetUniformLocation		= (PFNGLGETUNIFORMLOCATIONPROC)			SDL_GL_GetProcAddress("glGetUniformLocation");
	glUniform4f					= (PFNGLUNIFORM4FPROC) 					SDL_GL_GetProcAddress("glUniform4f");
	glUniform3f					= (PFNGLUNIFORM3FPROC) 					SDL_GL_GetProcAddress("glUniform3f");
	glCreateShader				= (PFNGLCREATESHADERPROC)				SDL_GL_GetProcAddress("glCreateShader");
	glShaderSource				= (PFNGLSHADERSOURCEPROC)				SDL_GL_GetProcAddress("glShaderSource");
	glCompileShader				= (PFNGLCOMPILESHADERPROC)				SDL_GL_GetProcAddress("glCompileShader");
	glCreateProgram				= (PFNGLCREATEPROGRAMPROC)				SDL_GL_GetProcAddress("glCreateProgram");
	glDeleteShader				= (PFNGLDELETESHADERPROC)				SDL_GL_GetProcAddress("glDeleteShader");
	glAttachShader				= (PFNGLATTACHSHADERPROC)				SDL_GL_GetProcAddress("glAttachShader");
	glLinkProgram				= (PFNGLLINKPROGRAMPROC)				SDL_GL_GetProcAddress("glLinkProgram");
	glGenVertexArrays			= (PFNGLGENVERTEXARRAYSPROC)			SDL_GL_GetProcAddress("glGenVertexArrays");
	glGenBuffers				= (PFNGLGENBUFFERSPROC)					SDL_GL_GetProcAddress("glGenBuffers");
	glDeleteVertexArrays		= (PFNGLDELETEVERTEXARRAYSPROC)			SDL_GL_GetProcAddress("glDeleteVertexArrays");
	glDeleteProgram				= (PFNGLDELETEPROGRAMPROC)				SDL_GL_GetProcAddress("glDeleteProgram");
	_glActiveTexture			= (PFNGLACTIVETEXTUREPROC)				SDL_GL_GetProcAddress("glActiveTexture");
	glUniform1i					= (PFNGLUNIFORM1IPROC)					SDL_GL_GetProcAddress("glUniform1i");
	glDisableVertexAttribArray  = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)   SDL_GL_GetProcAddress("glDisableVertexAttribArray");
	glUniform1f					= (PFNGLUNIFORM1FPROC)					SDL_GL_GetProcAddress("glUniform1f");
	glBindSampler				= (PFNGLBINDSAMPLERPROC)				SDL_GL_GetProcAddress("glBindSampler");
	glBlendEquationSeparate		= (PFNGLBLENDEQUATIONSEPARATEPROC)		SDL_GL_GetProcAddress("glBlendEquationSeparate");
	glBlendFuncSeparate			= (PFNGLBLENDFUNCSEPARATEPROC) 			SDL_GL_GetProcAddress("glBlendFuncSeparate");
	glGetAttribLocation 		= (PFNGLGETATTRIBLOCATIONPROC)			SDL_GL_GetProcAddress("glGetAttribLocation");
	glDetachShader				= (PFNGLDETACHSHADERPROC)				SDL_GL_GetProcAddress("glDetachShader");
	_glBlendEquation			= (PFNGLBLENDEQUATIONPROC)				SDL_GL_GetProcAddress("glBlendEquation");
}

const GLchar* graph_vertex = R"STR(
	#version 330 core

	layout (location = 0) in vec3 position;
	
	uniform mat4 modelviewproj;
	
	void main() {
		gl_Position = modelviewproj * vec4(position, 1.0f);
	}
)STR";

const GLchar* graph_fragment = R"STR(
	#version 330 core

	uniform vec4 vcolor;
	out vec4 color;

	void main() {
		color = vcolor;
	}
)STR";

const GLchar* graph_vertex_lighting = R"STR(
	#version 330 core

	layout (location = 0) in vec3 position;
	layout (location = 1) in vec3 normal;

	uniform mat4 modelviewproj;
	uniform mat4 model;
	
	out vec3 norm;
	out vec3 fragPos;

	void main() {
		gl_Position = modelviewproj * vec4(position, 1.0f);
		norm = vec3(model * vec4(normal, 1.0f));
		fragPos = vec3(model * vec4(position, 1.0f));
	}
)STR";

const GLchar* graph_fragment_lighting = R"STR(
	#version 330 core

	uniform vec4 vcolor;
	uniform vec3 lightColor;
	uniform vec3 lightPos;
	uniform float ambientStrength;
	uniform bool use_norm;

	in vec3 norm;
	in vec3 fragPos;
	out vec4 color;

	void main() {
		vec3 ambient = ambientStrength * lightColor;
		vec3 lightDir = normalize(lightPos - fragPos);
		float diff = abs(dot(norm, lightDir));
		vec3 diffuse = diff * lightColor;
		color = vec4(ambient + diffuse, 1.0f) * vcolor;
	}
)STR";

const GLchar* graph_vertex_norm = R"STR(
	#version 330 core

	layout (location = 0) in vec3 position;
	layout (location = 1) in vec3 normal;

	uniform mat4 modelviewproj;
	uniform mat4 model;
	
	out vec3 norm;
	out vec3 fragPos;

	void main() {
		gl_Position = modelviewproj * vec4(position, 1.0f);
		norm = vec3(model * vec4(normal, 1.0f));
		fragPos = vec3(model * vec4(position, 1.0f));
	}
)STR";

const GLchar* graph_fragment_norm = R"STR(
	#version 330 core

	uniform vec4 vcolor;
	uniform vec3 lightColor;
	uniform vec3 lightPos;
	uniform float ambientStrength;

	in vec3 norm;
	in vec3 fragPos;
	out vec4 color;

	void main() {
		vec3 ambient = ambientStrength * lightColor;
		vec3 lightDir = normalize(lightPos - fragPos);
		float diff = abs(dot(norm, lightDir));
		vec3 diffuse = diff * lightColor;
		color = vec4(ambient + diffuse, 1.0f) * abs(vec4(norm, 1.0));
	}
)STR";

const GLchar* axis_vertex = R"STR(
	#version 330 core

	layout (location = 0) in vec3 position;
	layout (location = 1) in vec3 color;

	out vec3 vcolor;

	uniform mat4 modelviewproj;

	void main() {
		gl_Position = modelviewproj * vec4(position, 1.0f);
		vcolor = color;
	}
)STR";

const GLchar* axis_fragment = R"STR(
	#version 330 core

	in vec3 vcolor;
	out vec4 color;

	void main() {
		color = vec4(vcolor, 1.0f);
	}
)STR";
