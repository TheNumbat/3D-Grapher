
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

void debug_proc(GLenum glsource, GLenum gltype, GLuint, GLenum severity, GLsizei, const GLchar* glmessage, const void*) {

	std::string message((char*)glmessage);
	std::string source, type;

	switch(glsource) {
	case GL_DEBUG_SOURCE_API:
		source = "OpenGL API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		source = "Window System";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		source = "Shader Compiler";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		source = "Third Party";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		source = "Application";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		source = "Other";
		break;
	}

	switch(gltype) {
	case GL_DEBUG_TYPE_ERROR:
		type = "Error";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		type = "Deprecated";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		type = "Undefined Behavior";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		type = "Portability";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		type = "Performance";
		break;
	case GL_DEBUG_TYPE_MARKER:
		type = "Marker";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		type = "Push Group";
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		type = "Pop Group";
		break;
	case GL_DEBUG_TYPE_OTHER:
		type = "Other";
		break;
	}

	switch(severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		std::cout << "HIGH OpenGL: " << message << " SOURCE: " << source << " TYPE: " << type << std::endl;
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		std::cout << "MED OpenGL: " << message << " SOURCE: " << source << " TYPE: " << type << std::endl;
		break;
	case GL_DEBUG_SEVERITY_LOW:
		std::cout << "LOW OpenGL: " << message << " SOURCE: " << source << " TYPE: " << type << std::endl;
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		break;
	}
}

const GLchar* graph_vertex = R"STR(
	#version 330 core

	layout (location = 0) in vec3 v_pos;
	layout (location = 1) in vec3 v_norm;
	layout (location = 2) in vec3 v_color;
	
	uniform mat4 modelviewproj;
	out vec3 f_color;
	
	void main() {
		gl_Position = modelviewproj * vec4(v_pos, 1.0f);
		f_color = v_color;
	}
)STR";

const GLchar* graph_fragment = R"STR(
	#version 330 core

	in vec3 f_color;

	uniform float opacity;

	out vec4 out_color;

	void main() {
		out_color = vec4(f_color, opacity);
	}
)STR";

const GLchar* graph_vertex_lighting = R"STR(
	#version 330 core

	layout (location = 0) in vec3 v_pos;
	layout (location = 1) in vec3 v_norm;
	layout (location = 2) in vec3 v_color;

	uniform mat4 modelviewproj;
	uniform mat4 model;
	
	out vec3 f_pos;
	out vec3 f_norm;
	out vec3 f_color;

	void main() {
		gl_Position = modelviewproj * vec4(v_pos, 1.0);
		f_norm = vec3(model * vec4(v_norm, 1.0));
		f_color = v_color;
		f_pos = vec3(model * vec4(v_pos, 1.0));
	}
)STR";

const GLchar* graph_fragment_lighting = R"STR(
	#version 330 core

	uniform vec3 lightColor;
	uniform vec3 lightPos;
	uniform float ambientStrength;
	uniform float opacity;

	in vec3 f_pos;
	in vec3 f_norm;
	in vec3 f_color;
	out vec4 out_color;

	void main() {
		vec3 ambient = ambientStrength * lightColor;
		vec3 lightDir = normalize(lightPos - f_pos);
		float diff = abs(dot(f_norm, lightDir));
		vec3 diffuse = diff * lightColor;
		out_color = vec4(ambient + diffuse, 1.0f) * vec4(f_color, opacity);
	}
)STR";

const GLchar* axis_vertex = R"STR(
	#version 330 core

	layout (location = 0) in vec3 v_pos;
	layout (location = 1) in vec3 v_color;

	out vec3 f_color;

	uniform mat4 modelviewproj;

	void main() {
		gl_Position = modelviewproj * vec4(v_pos, 1.0f);
		f_color = v_color;
	}
)STR";

const GLchar* axis_fragment = R"STR(
	#version 330 core

	in vec3 f_color;
	out vec4 out_color;

	void main() {
		out_color = vec4(f_color, 1.0f);
	}
)STR";
