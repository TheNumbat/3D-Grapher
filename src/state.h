
#pragma once

const int screen_w = 1280, screen_h = 720;

struct state {
	state();
	~state();
	void run();

	void Events();
	void UI();
	void RenderAxes();
	void RenderGraphs();
	mat4 modelviewproj;

	enum class mode {
		idle,
		cam
	};

	// window
	SDL_Window* window;
	SDL_GLContext context;
	const unsigned char* keys;

	// input & state
	int w, h, mx, my, next_graph_id, last_mx, last_my;
	bool running;
	mode current;

	// errors
	bool error_shown;
	string error;

	// OpenGL
	GLuint axisVAO, axisVBO;
	shader graph_s, axis_s, UI_s, rect_s, graph_s_light;
	
	// Math
	vector<graph*> graphs;

	// camera
	cam_type camtype = cam_3d_static;
	_cam_3d c_3d;
	_cam_3d_static c_3d_static;
};
