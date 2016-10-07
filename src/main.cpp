#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <assert.h>

#include <SDL.h>
#include <SDL_image.h>
//#define GL3_PROTOTYPES 1
#include <GL\glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

const int width = 1280, height = 720;

SDL_Window* window;
SDL_GLContext context;
SDL_Surface *bricks, *face;
GLuint program, btex, ftex; // programOrange, programBlue;

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
	 // vert				// color			// tex coord
	-0.5f,  0.0f, 0.0f,		1.0f, 0.0f, 0.0f,	0.0f, 1.0f,
	 0.0f, -0.5f, 0.0f,		0.0f, 1.0f, 0.0f,	1.0f, 0.0f,
	 0.0f,  0.0f, 0.0f,		0.0f, 0.0f, 1.0f,	1.0f, 1.0f,
	 //
	 0.25f, 0.25f, 0.0f,	1.0f, 0.0f, 0.0f,	0.0f, 0.0f,
	 0.5f , 0.25f, 0.0f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f,
	 0.25f, 0.5f , 0.0f,	0.0f, 0.0f, 1.0f,	0.0f, 1.0f,
	 //
	 0.5f, 0.0f, 0.0f,		1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
	 0.0f, 0.5f, 0.0f,		0.0f, 1.0f, 0.0f,	0.0f, 1.0f,
	 0.0f, 0.0f, 0.0f,		0.0f, 0.0f, 1.0f,	0.0f, 0.0f,
	 //
	 -0.25f, -0.25f, 0.0f,	1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
	 -0.5f , -0.25f, 0.0f,	0.0f, 1.0f, 0.0f,	0.0f, 1.0f,
	 -0.25f, -0.5f , 0.0f,	0.0f, 0.0f, 1.0f,	1.0f, 0.0f
};

/*
GLfloat vertices2[] = {
	0.0f, 0.0f, 0.0f,
	0.0f, 0.5f, 0.0f,
	0.5f, 0.0f, 0.0f,
	//
	-0.25f, -0.25f, 0.0f,
	-0.5f , -0.25f, 0.0f,
	-0.25f, -0.5f , 0.0f
}; */

int main(int argc, char** args) {

	init();
	shaders();

	GLuint VBOid1;// VBOid2;
	glGenBuffers(1, &VBOid1);
	//glGenBuffers(1, &VBOid2);

	//GLuint EBOid;
	//glGenBuffers(1, &EBOid);

	GLuint VAOid1;// VAOid2;
	glGenVertexArrays(1, &VAOid1);
	//glGenVertexArrays(1, &VAOid2);

	glBindVertexArray(VAOid1);

		glBindBuffer(GL_ARRAY_BUFFER, VBOid1);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOid);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	/*glBindVertexArray(VAOid2);

		glBindBuffer(GL_ARRAY_BUFFER, VBOid2);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

	glBindVertexArray(0);*/

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	bool running = true;
	SDL_Event ev;
	while (running) {
		glClearColor(1.0, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glUseProgram(program);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, btex);
		glUniform1i(glGetUniformLocation(program, "tex1"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, ftex);
		glUniform1i(glGetUniformLocation(program, "tex2"), 1);

		mat4 model, view, persp;
		model = rotate(model, radians(-45.0f), vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, value_ptr(model));

		glBindVertexArray(VAOid1);
			glDrawArrays(GL_TRIANGLES, 0, 12);
			//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		//GLfloat time = SDL_GetTicks() / 500.0f;
		//GLfloat green = sin(time) / 2 + 0.5;
		//GLint pcolorLocation = glGetUniformLocation(programBlue, "pcolor");
		//glUseProgram(programBlue);
		//glUniform4f(pcolorLocation, 0.0f, green, 0.0f, 1.0f);
		//glBindVertexArray(VAOid2);
			//glDrawArrays(GL_TRIANGLES, 0, 6);
		//glBindVertexArray(0);

		SDL_GL_SwapWindow(window);

		while (SDL_PollEvent(&ev) != 0)
		{
			if (ev.type == SDL_QUIT)
				running = false;
		}
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_FreeSurface(bricks);
	IMG_Quit();
	SDL_Quit();
	return 0;
}

void shaders() {
	GLuint vert, frag;

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
	fin.open("fragment.glsl");
	while (!fin.eof()) {
		std::getline(fin, line);
		file.append(line + "\n");
	}
	fin.close();

	source = file.c_str();
	frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag, 1, &source, NULL);
	glCompileShader(frag);

	/*file = "";
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
	glLinkProgram(programBlue);*/

	program = glCreateProgram();
	glAttachShader(program, vert);
	glAttachShader(program, frag);
	glLinkProgram(program);

	glDeleteShader(vert);
	glDeleteShader(frag);
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

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
	bricks = IMG_Load("wall.jpg");
	SDL_ConvertSurfaceFormat(bricks, SDL_PIXELFORMAT_RGB888, 0);
	face = IMG_Load("face.png");
	SDL_ConvertSurfaceFormat(face, SDL_PIXELFORMAT_RGBA8888, 0);

	glGenTextures(1, &btex);
	glGenTextures(1, &ftex);
	glBindTexture(GL_TEXTURE_2D, btex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bricks->w, bricks->h, 0, GL_RGB, GL_UNSIGNED_BYTE, bricks->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ftex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, face->w, face->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, face->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glViewport(0, 0, width, height);
}