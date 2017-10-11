
#pragma once

const int screen_w = 1280, screen_h = 720;

struct state {
	state();
	~state();
	void run();

	void Events();
	
	void RenderAxes();
	void RenderGraphs();

	void UI();
	void UIGraphs();
	void UICamera();
	void UIFunc();
	void UISettings();
	void UIError();
	void UIHelp();
	
	void updateAxes();
	void resetCam();
	void regengraph(int index);
	void regenall();

	// input & state
	enum class mode {
		idle,
		cam
	};
	struct uistate {
		bool cam = false, func = false, settings = false, error_shown = false, help = false;
		unsigned int settings_index = 0;
		string error;
		mode current = mode::idle;
	};

	uistate ui;
	int w, h, mx, my, next_graph_id, last_mx, last_my;
	bool running;

	// window
	SDL_Window* window;
	SDL_GLContext context;
	const unsigned char* keys;

	// Math
	vector<graph*> graphs;
	GLfloat axes[36] = {};

	// OpenGL
	GLuint axisVAO = 0, axisVBO = 0;
	shader graph_s, axis_s, UI_s, rect_s, graph_s_light, graph_s_norm;
	mat4 modelviewproj;

	// camera
	cam_type camtype = cam_3d_static;
	_cam_3d c_3d;
	_cam_3d_static c_3d_static;
};
