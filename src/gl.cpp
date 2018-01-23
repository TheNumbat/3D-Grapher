
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

	check(v);
	check(f);

	program = glCreateProgram();
	glAttachShader(program, v);
	glAttachShader(program, f);
	glLinkProgram(program);
	glDeleteShader(v);
	glDeleteShader(f);
}

void shader::check(GLuint program) {

	GLint isCompiled = 0;
	glGetShaderiv(program, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE) {
		
		GLint len = 0;
		glGetShaderiv(program, GL_INFO_LOG_LENGTH, &len);

		char* msg = new char[len];
		glGetShaderInfoLog(program, len, &len, msg);

		std::cout << "Shader  " << program << " failed to compile: " << msg << std::endl;
		delete[] msg;
	}
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

	uniform mat4 viewproj;
	uniform int highlight;

	out vec3 f_pos;
	out vec3 f_norm;
	out vec3 f_color;

	mat4 rotMat(vec3 axis, float angle) {
	    axis = normalize(axis);
	    float s = sin(angle);
	    float c = cos(angle);
	    float oc = 1.0 - c;
	    
	    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
	                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
	                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
	                0.0,                                0.0,                                0.0,                                1.0);
	}

	void main() {
		mat4 rot = rotMat(vec3(1, 0, 0), -1.5708);
		vec3 v_out = v_pos;
		
		f_pos = vec3(rot * vec4(v_out, 1.0f));
		f_norm = vec3(rot * vec4(v_norm, 1.0));
		f_color = v_color;
		
		if(highlight != 0) v_out[highlight - 1] = 0;
		gl_Position = viewproj * rot * vec4(v_out, 1.0f);
	}
)STR";

const GLchar* graph_fragment = R"STR(
	#version 330 core

	uniform vec3 lightColor;
	uniform vec3 lightPos;
	uniform float ambientStrength;
	uniform float opacity;

	uniform bool lighting;
	uniform int highlight;
	uniform vec3 highlight_pos;
	uniform vec3 highlight_color;
	uniform float tolerance;

	in vec3 f_pos;
	in vec3 f_norm;
	in vec3 f_color;
	out vec4 out_color;

	bool approx(float l, float r) {
		return l > r - tolerance && l < r + tolerance;
	}

	void main() {
		vec3 c = f_color;
		if(highlight != 0) {
			int dim = highlight - 1;
			float h = highlight_pos[dim];
			if(dim == 1) dim = 2;
			else if(dim == 2) dim = 1;
			float p = f_pos[dim];
			if(approx(p, h)) {
				c = highlight_color;
			} else {
				discard;
			}
		}
		if(lighting) {
			vec3 ambient = ambientStrength * lightColor;
			vec3 lightDir = normalize(lightPos - f_pos);
			float diff = abs(dot(f_norm, lightDir));
			vec3 diffuse = diff * lightColor;
			c *= ambient + diffuse;
		}
		out_color = vec4(c, opacity);
	}
)STR";

const GLchar* axis_vertex = R"STR(
	#version 330 core

	layout (location = 0) in vec3 v_pos;
	layout (location = 1) in vec3 v_color;

	out vec3 f_color;

	uniform mat4 viewproj;

	mat4 rotMat(vec3 axis, float angle) {
	    axis = normalize(axis);
	    float s = sin(angle);
	    float c = cos(angle);
	    float oc = 1.0 - c;
	    
	    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
	                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
	                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
	                0.0,                                0.0,                                0.0,                                1.0);
	}

	void main() {
		gl_Position = viewproj * rotMat(vec3(1,0,0), -1.5708) * vec4(v_pos, 1.0f);
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
