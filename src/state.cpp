
state::state() {
	w = screen_w;
	h = screen_h;
	mx = w / 2;
	my = h / 2;
	next_graph_id = 0;
	last_mx = last_my = 0;

	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	window = SDL_CreateWindow("3D Grapher", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screen_w, screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	context = SDL_GL_CreateContext(window);

	setupFuns();
	SDL_GL_SetSwapInterval(0);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, w, h);

	glGenVertexArrays(1, &axisVAO);
	glGenBuffers(1, &axisVBO);

	graph_s.load(graph_vertex, graph_fragment);
	axis_s.load(axis_vertex, axis_fragment);
	graph_s_light.load(graph_vertex_lighting, graph_fragment_lighting);

	ImGui_ImplSdlGL3_Init(window);
	ImGui::GetStyle().WindowRounding = 0.0f;

	ImGuiIO& io = ImGui::GetIO();
	static const ImWchar range[] = {32, 127, 215, 215, 913, 969, 8592, 9654, 0}; // apparently this needs to be persistent...
	io.Fonts->AddFontFromFileTTF("font.ttf", 18, 0, range);

	updateAxes(this);

	keys = SDL_GetKeyboardState(NULL);
	current = mode::idle;

	c_3d.reset();
	c_3d_static.reset();
	running = true;
}

state::~state() {
	for (graph* g : graphs)
		delete g;
	ImGui_ImplSdlGL3_Shutdown();
	glDeleteBuffers(1, &axisVBO);
	glDeleteVertexArrays(1, &axisVAO);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void state::run() {

	while (running) {
		Uint64 start = SDL_GetPerformanceCounter();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		RenderGraphs();
		RenderAxes();
		UI();
		Events();

		SDL_GL_SwapWindow(window);

		Uint64 end = SDL_GetPerformanceCounter();
		double micro = 1000000.0f * (end - start) / (float)SDL_GetPerformanceFrequency();
		std::this_thread::sleep_for(std::chrono::microseconds(16666 - (Uint64)micro));
	}
}

void state::RenderGraphs() {

	mat4 model, view, proj;
	model = rotate(model, radians(-90.0f), vec3(1, 0, 0));
	if (set.camtype == cam_3d) {
		view = c_3d.getView();
		proj = perspective(radians(set.fov), (GLfloat)w / (GLfloat)h, 0.1f, 1000.0f);
	}
	else {
		view = c_3d_static.getView();
		proj = perspective(radians(set.fov), (GLfloat)w / (GLfloat)h, 0.1f, 1000.0f);
	} 
	modelviewproj =  proj * view * model;

	for (graph* g : graphs) {
		g->draw(this, model, view, proj);
	}
}

void state::RenderAxes() {
	glBindVertexArray(axisVAO);
	{
		axis_s.use();

		glBindBuffer(GL_ARRAY_BUFFER, axisVBO);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		glUniformMatrix4fv(axis_s.getUniform("modelviewproj"), 1, GL_FALSE, value_ptr(modelviewproj));

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		glDrawArrays(GL_LINES, 0, 6);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}
	glBindVertexArray(0);
}

void state::UI() {

	ImGui_ImplSdlGL3_NewFrame(window);

	ImGui::SetNextWindowPos({0, 0});
	ImGui::SetNextWindowSize({0.2f * w, (float)h});
	ImGui::Begin("Main", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	
	static bool func = false;
	if(ImGui::Button("Add a Function")) {
		func = !func;
	}

	if(func) {
		ImGui::SetNextWindowFocus();
		ImGui::Begin("Add a Function", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

		if(ImGui::Button("Rectangular")) {
			func = false;
			graphs.push_back(new fxy_graph(next_graph_id++));
		}
		if(ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
				ImGui::Text("z = f(x,y)");
			ImGui::EndTooltip();
		}

		if(ImGui::Button("Cylindrical")) {
			func = false;
			graphs.push_back(new cyl_graph(next_graph_id++));
		}
		if(ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
				ImGui::Text("z = ψ(r,θ)");
			ImGui::EndTooltip();
		}

		if(ImGui::Button("Spherical")) {
			func = false;
			graphs.push_back(new spr_graph(next_graph_id++));
		}
		if(ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
				ImGui::Text("r = ρ(θ,φ)");
			ImGui::EndTooltip();
		}

		if(ImGui::Button("Parametric Curve")) {
			func = false;
			graphs.push_back(new para_curve(next_graph_id++));
		}
		if(ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
				ImGui::Text("x = f(t)");
				ImGui::Text("y = g(t)");
				ImGui::Text("z = h(t)");
			ImGui::EndTooltip();
		}

		if(ImGui::Button("Cancel")) {
			func = false;
		}

		ImGui::End();
	}

	ImGui::Separator();
	ImGui::Columns(2);
	ImGui::SetColumnWidth(-1, ImGui::GetWindowWidth() * 0.85f);

	for(int i = 0; i < graphs.size(); i++) {
		graph* g = graphs[i];

		if(ImGui::InputTextMultiline(to_string(g->ID).c_str(), (char*)g->eq_str.c_str(), 1000, ImVec2(ImGui::GetColumnWidth() - 25, 50), ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue)) {
			regengraph(this, getIndex(this, g->ID));		
		}
		ImGui::NextColumn();
		if(ImGui::Button("×")) {
			delete g;
			graphs.erase(graphs.begin() + i);
			i--;
		}
		if(ImGui::Button("▶")) {
			regengraph(this, getIndex(this, g->ID));
		}
		ImGui::NextColumn();

		ImGui::Separator();
	}

	ImGui::End();
	ImGui::Render();
}

void state::Events() {

	auto& io = ImGui::GetIO();

	SDL_Event e;
	while(SDL_PollEvent(&e) != 0) {

		ImGui_ImplSdlGL3_ProcessEvent(&e);

		switch(e.type) {
		case SDL_QUIT: {
			running = false;
		} break;

		case SDL_WINDOWEVENT: {
			if (e.window.event == SDL_WINDOWEVENT_RESIZED ||
				e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
				w = e.window.data1;
				h = e.window.data2;
				glViewport(0, 0, w, h);
				glScissor(0, 0, w, h);
			}
		} break;

		case SDL_KEYDOWN: {
			if(e.key.keysym.sym == SDLK_ESCAPE) {
				running = false;
			}
		} break;

		case SDL_MOUSEMOTION: {
			if(current == mode::cam) {
				int dx = (e.motion.x - mx);
				int dy = (e.motion.y - my);
				if (set.camtype == cam_3d) {
					c_3d.move(dx, dy);
				}
				else if (set.camtype == cam_3d_static) {
					c_3d_static.move(dx, dy);
				}
			}
			mx = e.motion.x;
			my = e.motion.y;
		} break;

		case SDL_MOUSEBUTTONDOWN: {
			if(!io.WantCaptureMouse) {
				current = mode::cam;
				last_mx = e.button.x;
				last_my = e.button.y;
				SDL_CaptureMouse(SDL_TRUE);
				SDL_SetRelativeMouseMode(SDL_TRUE);
			}
		} break;
		case SDL_MOUSEBUTTONUP: {
			if(!io.WantCaptureMouse) {
				current = mode::idle;
				SDL_CaptureMouse(SDL_FALSE);
				SDL_SetRelativeMouseMode(SDL_FALSE);
				SDL_WarpMouseInWindow(window, last_mx, last_my);
			}
		} break;
		}
	}

	float dT = (SDL_GetTicks() - c_3d.lastUpdate) / 1000.0f;
	c_3d.lastUpdate = SDL_GetTicks();
	if (set.camtype == cam_3d && current == mode::cam) {
		if (keys[SDL_SCANCODE_W]) {
			c_3d.pos += c_3d.front * c_3d.speed * dT;
		}
		if (keys[SDL_SCANCODE_S]) {
			c_3d.pos -= c_3d.front * c_3d.speed * dT;
		}
		if (keys[SDL_SCANCODE_A]) {
			c_3d.pos -= c_3d.right * c_3d.speed * dT;
		}
		if (keys[SDL_SCANCODE_D]) {
			c_3d.pos += c_3d.right * c_3d.speed * dT;
		}
	}
}
