#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <assert.h>
#include "cam.cpp"
#include <SDL.h>
#include <SDL_image.h>
//#define GL3_PROTOTYPES 1
#include <GL/glew.h>
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

/*GLfloat vertices1[] = {
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
};*/

GLfloat bar[] {
	-0.25f, -1.0f, 0.0f, 0.0f, 0.0f,
	-0.25f, 1.0f, 0.0f, 0.0f, 0.0f,
	0.25f, 1.0f, 0.0f, 0.0f, 0.0f,

	0.25f, 1.0f, 0.0f, 0.0f, 0.0f,
	-0.25f, -1.0f, 0.0f, 0.0f, 0.0f,
	0.25f, -1.0f, 0.0f, 0.0f, 0.0f
};

GLfloat vertices[] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

vec3 cubePositions[] = {
	vec3(1.0f,  1.0f,  1.0f),
	vec3(2.0f,  5.0f, -15.0f),
	vec3(-1.5f, -2.2f, -2.5f),
	vec3(-3.8f, -2.0f, -12.3f),
	vec3(2.4f, -0.4f, -3.5f),
	vec3(-1.7f,  3.0f, -7.5f),
	vec3(1.3f, -2.0f, -2.5f),
	vec3(1.5f,  2.0f, -2.5f),
	vec3(1.5f,  0.2f, -1.5f),
	vec3(-1.3f,  1.0f, -1.5f)
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
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOid);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		//glEnableVertexAttribArray(1);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	glBindVertexArray(VAOid2);

		glBindBuffer(GL_ARRAY_BUFFER, VBOid2);
		glBufferData(GL_ARRAY_BUFFER, sizeof(bar), bar, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	float mx = 0, my = 0, mdxa = 0, mdya = 0;
	//GLfloat cameraSpeed = 0.1f, pitch = 0, yaw = 0, roll = 0;
	//vec3 cameraPos, cameraFront, cameraTarget, cameraRight, cameraUp;
	Camera cam;
	bool running = true;
	SDL_Event ev;
	GLuint time = SDL_GetTicks();
	while (running) {
		while (SDL_PollEvent(&ev) != 0)
		{
			if (ev.type == SDL_QUIT)
				running = false;
			if (ev.type == SDL_MOUSEMOTION) {
				float x = ev.motion.x, y = ev.motion.y;
				float dx = mx - x - mdxa, dy = my - y - mdya;
				cam.ProcessMouseMovement(dx, -dy, false);
				//pitch += dy / 50.0f;
				//yaw -= dx / 50.0f;
				SDL_WarpMouseInWindow(window, width/2, height/2);
				mdxa = width/2 - x;
				mdya = height/2 - y;
				mx = x; my = y;
			}
		}
		
		mat4 proj, view = cam.GetViewMatrix();
		proj = perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
		
		
	/*	GLfloat mat[] = { cos(yaw)*cos(pitch), -cos(yaw)*sin(pitch)*sin(roll) - sin(yaw)*cos(roll), -cos(yaw)*sin(pitch)*cos(roll) + sin(yaw)*sin(roll),
						  sin(yaw)*cos(pitch), -sin(yaw)*sin(pitch)*sin(roll) + cos(yaw)*cos(roll), -sin(yaw)*sin(pitch)*cos(roll) - cos(yaw)*sin(roll),
						  sin(pitch),			cos(pitch)*sin(roll),								cos(pitch)*sin(roll) }; */
		//mat3 rot = make_mat3(mat);
		//cameraFront = normalize(rot * vec3(0,0,1));
		//vec3 dir(sin(yaw), -(sin(pitch)*cos(yaw)), -(cos(pitch)*cos(yaw)));
		
		//glm::vec3 front;
		//front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		//front.y = sin(glm::radians(pitch));
		//front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		//cameraFront = glm::normalize(front);

		//cameraTarget = cameraPos + cameraFront;
		//cameraRight = normalize(cross(vec3(0.0f, 1.0f, 0.0f), cameraTarget));
		//cameraUp = normalize(cross(cameraTarget, cameraRight));
		//cameraUp = vec3(0.0f, 1.0f, 0.0f);

		//view = lookAt(cameraPos, cameraTarget, vec3(0.0f, 1.0f, 0.0f));

		const unsigned char* keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_W]) {
			//cameraPos += cameraSpeed * cameraFront;
			cam.ProcessKeyboard(FORWARD, (SDL_GetTicks() - time) / 1000.0f);
		}
		if (keys[SDL_SCANCODE_S]) {
			//cameraPos -= cameraSpeed * cameraFront;
			cam.ProcessKeyboard(BACKWARD, (SDL_GetTicks() - time) / 1000.0f);
		}
		if (keys[SDL_SCANCODE_A]) {
			//cameraPos -= normalize(cross(cameraFront, vec3(0.0f, 1.0f, 0.0f))) * cameraSpeed;
			cam.ProcessKeyboard(LEFT, (SDL_GetTicks() - time) / 1000.0f);
		}
		if (keys[SDL_SCANCODE_D]) {
			//cameraPos += normalize(cross(cameraFront, vec3(0.0f, 1.0f, 0.0f))) * cameraSpeed;
			cam.ProcessKeyboard(RIGHT, (SDL_GetTicks() - time) / 1000.0f);
		}
		time = SDL_GetTicks();

		glClearColor(1.0, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glUseProgram(program);

		//cameraPos = vec3(sin(SDL_GetTicks() / 500.0f) * 5.0f, 0.0f, cos(SDL_GetTicks() / 500.0f) * 5.0f);
		//pitch = sin(SDL_GetTicks() / 500.0f) * 89.0f;
		//yaw = sin(SDL_GetTicks() / 500.0f) * 89.0f;
		//cameraFront = vec3(0.0f, 0.0f, 1.0f);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, btex);
		glUniform1i(glGetUniformLocation(program, "tex1"), 0);
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, ftex);
		//glUniform1i(glGetUniformLocation(program, "tex2"), 1);

		glUniformMatrix4fv(glGetUniformLocation(program, "proj"), 1, GL_FALSE, value_ptr(proj));
		glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, value_ptr(view));

		glBindVertexArray(VAOid1);
		for (GLuint i = 0; i < 10; i++)
		{
			mat4 model;
			model = translate(model, cubePositions[i]);
			model = rotate(model, SDL_GetTicks() / 1500.0f * (i + 1), vec3(1.0f, 0.3f, 0.5f));
		
			//view = rotate(view, SDL_GetTicks() / 500.0f, vec3(0.0f, 1.0f, 0.0f));
			//float fov = sin(SDL_GetTicks() / 500.0f) * 90;
		
			glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, value_ptr(model));

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glBindVertexArray(VAOid2);
			//view = mat4(1);
			proj = ortho(0.0f, 0.0f, (GLfloat)width, (GLfloat)height, -1.0f, 1.0f);
			//mat4 proj2 = ortho(-100.0 f, 100.0f, -100.0f, 100.0f, -0.1f, 0.1f);
			mat4 model;
			//model = scale(model, vec3(10.0f, 10.0f, 1.0f));
			glUniformMatrix4fv(glGetUniformLocation(program, "proj"), 1, GL_FALSE, value_ptr(proj));
			glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);
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
	std::ifstream fin("vertex_old.glsl");
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
	fin.open("fragment_old.glsl");
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

	SDL_ShowCursor(0);
	SDL_SetWindowGrab(window, SDL_TRUE);
	//assert(!SDL_SetRelativeMouseMode(SDL_TRUE));
	//SDL_ShowCursor(1);

	context = SDL_GL_CreateContext(window);
	assert(context);

	glewExperimental = GL_TRUE;
	glewInit();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glEnable(GL_DEPTH_TEST);

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