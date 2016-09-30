#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <assert.h>

#include <SDL.h>
#define GL3_PROTOTYPES 1
#include <GL\glew.h>

const int width = 1280, height = 720;

SDL_Window* window;
SDL_GLContext context;
GLuint programOrange, programBlue;

void init();
void shaders();

/* rectangle with EBO indices
GLfloat vertices[] = {
	 0.5f,  0.5f, 0.0f,  // Top Right
	 0.5f, -0.5f, 0.0f,  // Bottom Right
	-0.5f, -0.5f, 0.0f,  // Bottom Left
	-0.5f,  0.5f, 0.0f   // Top Left 
};
GLuint indices[] = {  // Note that we start from 0!
	0, 1, 3,   // First Triangle
	1, 2, 3    // Second Triangle
}; */

GLfloat vertices1[] = {
	-0.5f,  0.0f, 0.0f,
	 0.0f, -0.5f, 0.0f,
	 0.0f,  0.0f, 0.0f,
	 //
	 0.25f, 0.25f, 0.0f,
	 0.5f , 0.25f, 0.0f,
	 0.25f, 0.5f , 0.0f
};

GLfloat vertices2[] = {
	0.0f, 0.0f, 0.0f,
	0.0f, 0.5f, 0.0f,
	0.5f, 0.0f, 0.0f,
	//
	-0.25f, -0.25f, 0.0f,
	-0.5f , -0.25f, 0.0f,
	-0.25f, -0.5f , 0.0f
};

int main(int argc, char** args) {

	init();
	shaders();

	GLuint VBOid1, VBOid2;
	glGenBuffers(1, &VBOid1);
	glGenBuffers(1, &VBOid2);

	//GLuint EBOid;
	//glGenBuffers(1, &EBOid);

	GLuint VAOid1, VAOid2;
	glGenVertexArrays(1, &VAOid1);
	glGenVertexArrays(1, &VAOid2);

	glBindVertexArray(VAOid1);

		glBindBuffer(GL_ARRAY_BUFFER, VBOid1);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOid);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	glBindVertexArray(VAOid2);

		glBindBuffer(GL_ARRAY_BUFFER, VBOid2);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	bool running = true;
	SDL_Event ev;
	while (running) {
		glClearColor(1.0, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(programOrange);
		glBindVertexArray(VAOid1);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glUseProgram(programBlue);
		glBindVertexArray(VAOid2);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		SDL_GL_SwapWindow(window);

		while (SDL_PollEvent(&ev) != 0)
		{
			if (ev.type == SDL_QUIT)
				running = false;
		}
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

void shaders() {
	GLuint vert, fragOrange, fragBlue;

	std::string file, line;
	std::ifstream fin("vertex.glsl");
	while (!fin.eof()) {
		std::getline(fin, line);
		file.append(line + "\n");
	}
	fin.close();

	const GLchar* source = file.c_str();
	vert = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert, 1, &source, NULL);
	glCompileShader(vert);

	file = "";
	fin.open("fragmentOrange.glsl");
	while (!fin.eof()) {
		std::getline(fin, line);
		file.append(line + "\n");
	}
	fin.close();

	source = file.c_str();
	fragOrange = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragOrange, 1, &source, NULL);
	glCompileShader(fragOrange);

	file = "";
	fin.open("fragmentBlue.glsl");
	while (!fin.eof()) {
		std::getline(fin, line);
		file.append(line + "\n");
	}
	fin.close();

	source = file.c_str();
	fragBlue = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragBlue, 1, &source, NULL);
	glCompileShader(fragBlue);

	programOrange = glCreateProgram();
	glAttachShader(programOrange, vert);
	glAttachShader(programOrange, fragOrange);
	glLinkProgram(programOrange);

	programBlue = glCreateProgram();
	glAttachShader(programBlue, vert);
	glAttachShader(programBlue, fragBlue);
	glLinkProgram(programBlue);

	glDeleteShader(vert);
	glDeleteShader(fragOrange);
	glDeleteShader(fragBlue);
}

void init() {
	window = SDL_CreateWindow("3D Grapher", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	assert(window);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetSwapInterval(1);

	context = SDL_GL_CreateContext(window);
	assert(context);

	glewExperimental = GL_TRUE;
	glewInit();

	glViewport(0, 0, width, height);
}