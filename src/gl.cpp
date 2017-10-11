
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
