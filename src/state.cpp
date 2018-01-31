
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
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	window = SDL_CreateWindow("3D Grapher", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screen_w, screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	context = SDL_GL_CreateContext(window);

	glewInit();
	SDL_GL_SetSwapInterval(1);

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(debug_proc, 0);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, w, h);

	glGenVertexArrays(1, &axisVAO);
	glGenBuffers(1, &axisVBO);

	axes[3] = axes[9] = axes[16] = axes[22] = axes[29] = axes[35] = 1.0f;
	updateAxes();

	graph_s.load(graph_vertex, graph_fragment);
	axis_s.load(axis_vertex, axis_fragment);

	ImGui_ImplSdlGL3_Init(window);
	ImGui::GetStyle().WindowRounding = 0.0f;

	ImGuiIO& io = ImGui::GetIO();
	static const ImWchar range[] = {32, 127, 215, 215, 913, 969, 8592, 9654, 9881, 9881, 0};
	io.Fonts->AddFontFromMemoryTTF(font_ttf, font_ttf_len, 18, 0, range);

	keys = SDL_GetKeyboardState(NULL);

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

void state::regengraph(int index) {

	Uint64 start = SDL_GetPerformanceCounter();
	graphs[index]->generate(this);
	Uint64 end = SDL_GetPerformanceCounter();
	std::cout << "time: " << (float)(end - start) / SDL_GetPerformanceFrequency() << std::endl;

	updateAxes();
	graphs[index]->send();
}

void state::regenall() {
	bool gen = false;
	for (int i = 0; i < (int)graphs.size(); i++) {
		regengraph(i);
		if (graphs[i]->verticies.size()) {
			gen = true;
		}
	}
	if (gen) {
		updateAxes();
	}
}

void state::updateAxes() {
	float xmin = FLT_MAX, xmax = -FLT_MAX;
	float ymin = FLT_MAX, ymax = -FLT_MAX;
	float zmin = FLT_MAX, zmax = -FLT_MAX;

	bool found_a_graph = false;
	for (graph* g : graphs) {
		if(g->verticies.size()) found_a_graph = true;
		if (g->zmin < zmin) zmin = g->zmin;
		if (g->zmax > zmax) zmax = g->zmax;
		if (g->ymin < ymin) ymin = g->ymin;
		if (g->ymax > ymax) ymax = g->ymax;
		if (g->xmin < xmin) xmin = g->xmin;
		if (g->xmax > xmax) xmax = g->xmax;			
	}
	if (zmin > 0) zmin = 0;
	if (zmax < 0) zmax = 0;
	if (ymin > 0) ymin = 0;
	if (ymax < 0) ymax = 0;
	if (xmin > 0) xmin = 0;
	if (xmax < 0) xmax = 0;
	
	if(!found_a_graph) {
		zmin = -10;
		zmax = 10;
		xmin = -10;
		xmax = 10;
		ymin = -10;
		ymax = 10;
	}

	axes[x_min] = xmin;
	axes[y_min] = ymin;
	axes[z_min] = zmin;
	axes[x_max] = xmax;
	axes[y_max] = ymax;
	axes[z_max] = zmax;

	glBindVertexArray(axisVAO); {
		
		glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

	} glBindVertexArray(0);
}

void state::resetCam() {
	c_3d_static.radius = std::max(axes[y_max] - axes[y_min], axes[x_max] - axes[x_min]);
	c_3d_static.lookingAt.x = (axes[x_max] + axes[x_min]) / 2;
	c_3d_static.lookingAt.z = (axes[y_max] + axes[y_min]) / -2;
	c_3d_static.updatePos();

	c_3d.pos.x = (axes[x_max] + axes[x_min]) / 2;
	c_3d.pos.z = (axes[y_max] + axes[y_min]) / -2;
	c_3d.updateFront();
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

	glm::mat4 view, proj;
	if (camtype == cam_type::_3d) {
		view = c_3d.getView();
		proj = glm::perspective(glm::radians(c_3d.fov), (GLfloat)w / (GLfloat)h, 0.1f, 1000.0f);
	}
	else {
		view = c_3d_static.getView();
		proj = glm::perspective(glm::radians(c_3d_static.fov), (GLfloat)w / (GLfloat)h, 0.1f, 1000.0f);
	} 
	viewproj =  proj * view;

	glEnable(GL_MULTISAMPLE);
	for (graph* g : graphs) {
		g->draw(this, viewproj);
	}
	glDisable(GL_MULTISAMPLE);
}

void state::RenderAxes() {
	
	glEnable(GL_MULTISAMPLE);
	glBindVertexArray(axisVAO); {
		axis_s.use();
		glUniformMatrix4fv(axis_s.getUniform("viewproj"), 1, GL_FALSE, value_ptr(viewproj));

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		glDrawArrays(GL_LINES, 0, 6);
	} glBindVertexArray(0);
	glDisable(GL_MULTISAMPLE);
}

int complete_callback(ImGuiTextEditCallbackData* data)
{
	int pos = data->CursorPos - 1;
	char* one = &data->Buf[pos];
	char* two = &data->Buf[pos - 1];

	if(*one == 'p') {
		data->DeleteChars(pos, 1);
		data->InsertChars(pos, "π");
		data->BufDirty = true;
	}
	else if (*one == 't') {
		data->DeleteChars(pos, 1);
		data->InsertChars(pos, "θ");
		data->BufDirty = true;
	}
	else if (strcmp(two, "π") == 0) {
		data->DeleteChars(pos - 1, 2);
		data->InsertChars(pos - 1, "φ");
		data->BufDirty = true;
	}
	else if (strcmp(two, "φ") == 0) {
		data->DeleteChars(pos - 1, 2);
		data->InsertChars(pos - 1, "p");
		data->BufDirty = true;
	}
	else if (strcmp(two, "θ") == 0) {
		data->DeleteChars(pos - 1, 2);
		data->InsertChars(pos - 1, "t");
		data->BufDirty = true;
	}

    return 0;
}

void state::UIGraphs() {
	using namespace ImGui;

	Columns(2);
	SetColumnWidth(-1, GetWindowWidth() * 0.85f);

	for(unsigned int i = 0; i < graphs.size(); i++) {
		graph* g = graphs[i];
		PushID(g->ID);

		switch(g->type) {
		case graph_func: {
			Text("f(x,y) =");
			if(InputTextMultiline("", (char*)g->eq_str.c_str(), 1000, ImVec2(GetColumnWidth() - 20, 60), text_input_flags, complete_callback)) {
				regengraph(i);
			}
		} break;
		case graph_cylindrical: {
			Text("ψ(z,θ) =");
			if(InputTextMultiline("", (char*)g->eq_str.c_str(), 1000, ImVec2(GetColumnWidth() - 20, 60), text_input_flags, complete_callback)) {
				regengraph(i);
			}
		} break;
		case graph_spherical: {
			Text("ρ(θ,φ) =");
			if(InputTextMultiline("", (char*)g->eq_str.c_str(), 1000, ImVec2(GetColumnWidth() - 20, 60), text_input_flags, complete_callback)) {
				regengraph(i);
			}
		} break;
		case graph_para_curve: {
			para_curve* p = (para_curve*)g;
			Text("x(t) =");
			PushID(0);
			InputTextMultiline("", (char*)p->eqx.c_str(), 1000, ImVec2(GetColumnWidth() - 20, 40), text_input_flags, complete_callback);
			PopID();
			Text("y(t) =");
			PushID(1);
			InputTextMultiline("", (char*)p->eqy.c_str(), 1000, ImVec2(GetColumnWidth() - 20, 40), text_input_flags, complete_callback);
			PopID();
			Text("z(t) =");
			PushID(2);
			InputTextMultiline("", (char*)p->eqz.c_str(), 1000, ImVec2(GetColumnWidth() - 20, 40), text_input_flags, complete_callback);
			PopID();
		} break;
		}
		switch(g->set.highlight_along) {
		case axis::x: {
			SliderFloat("X", &g->set.highlight_value.x, g->xmin, g->xmax);
		} break;
		case axis::y: {
			SliderFloat("Y", &g->set.highlight_value.y, g->ymin, g->ymax);
		} break;
		case axis::z: {
			SliderFloat("Z", &g->set.highlight_value.z, g->zmin, g->zmax);
		} break;
		}

		NextColumn();
		if(Button("×")) {
			if(ui.settings_index == i) {
				ui.settings_index = 0;
				ui.settings = false;
			}
			if(ui.settings_index > i) {
				ui.settings_index--;
			}
			delete g;
			graphs.erase(graphs.begin() + i);
			i--;
			updateAxes();
		}
		if(Button("▶")) {
			regengraph(i);
		}
		if(Button("⚙")) {
			if(ui.settings_index == i) ui.settings = !ui.settings;
			else ui.settings = true;
			ui.settings_index = i;
		}
		NextColumn();

		Separator();
		PopID();
	}
}

void state::UICamera() {
	using namespace ImGui;

	SetNextWindowPos({0.2f * w, 0.0f});
	Begin("Camera", &ui.cam, window_flags);
	
	const char* names[] = {"Free 3D", "Static 3D"};
	Combo("Camera", (int*)&camtype, names, 2);

	if(camtype == cam_type::_3d) {
		SliderFloat("FOV", &c_3d.fov, 10.0f, 170.0f);
	} else if(camtype == cam_type::_3d_static) {
		SliderFloat("FOV", &c_3d_static.fov, 10.0f, 170.0f);

		Text("Snap to:");
		SameLine();
		if(Button("XY")) {
			c_3d_static.setAxis(glm::vec3(0,0,1));
		}
		SameLine();
		if(Button("XZ")) {
			c_3d_static.setAxis(glm::vec3(0,1,0));	
		}
		SameLine();
		if(Button("YZ")) {
			c_3d_static.setAxis(glm::vec3(1,0,0));
		}
	}
	
	if(Button("Reset Camera")) {
		if(camtype == cam_type::_3d) {
			c_3d.reset();
		} else if(camtype == cam_type::_3d_static) {
			c_3d_static.reset();
		}
	}

	End();
}

void state::UIFunc() {
	using namespace ImGui;

	SetNextWindowPos({0.2f * w, 0.0f});
	Begin("Add a Graph", &ui.func, window_flags);

	if(Button("Rectangular")) {
		ui.func = false;
		graphs.push_back(new fxy_graph(next_graph_id++));
	}
	if(IsItemHovered()) {
		BeginTooltip();
			Text("f(x,y)");
		EndTooltip();
	}

	if(Button("Cylindrical")) {
		ui.func = false;
		graphs.push_back(new cyl_graph(next_graph_id++));
	}
	if(IsItemHovered()) {
		BeginTooltip();
			Text("ψ(z,θ)");
		EndTooltip();
	}

	if(Button("Spherical")) {
		ui.func = false;
		graphs.push_back(new spr_graph(next_graph_id++));
	}
	if(IsItemHovered()) {
		BeginTooltip();
			Text("ρ(θ,φ)");
		EndTooltip();
	}

	if(Button("Parametric Curve")) {
		ui.func = false;
		graphs.push_back(new para_curve(next_graph_id++));
	}
	if(IsItemHovered()) {
		BeginTooltip();
			Text("x = f(t)");
			Text("y = g(t)");
			Text("z = h(t)");
		EndTooltip();
	}

	End();
}

void state::UISettings() {
	using namespace ImGui;

	bool changed = false;
	graph* g = graphs[ui.settings_index];

	SetNextWindowPos({0.2f * w, 0.0f});
	Begin("Settings", &ui.settings, window_flags);
	
	static const char* calc_strings[] = {"None", "1st Partial, 1st Variable", "1st Partial, 2nd Variable", "2nd Partial, 1st Variable", "2nd Partial, 2nd Variable", "3rd Partial, 1st Variable", "3rd Partial, 2nd Variable"};
	static const char* para_strings[] = {"None", "1st Derivative", "2nd Derivative", "3rd Derivative"};
	static const char* highlight_strings[] = {"None", "X", "Y", "Z"};
	static const char* color_strings[] = {"Nothing", "Surface Normal", "Gradient"};

	if(g->type != graph_para_curve) {
		Checkbox("Wireframe", &g->set.wireframe);
		Checkbox("Lighting", &g->set.lighting);
		changed = changed || Checkbox("Normalization", &g->set.axisnormalization);
		changed = changed || Combo("Color By", (int*)&g->set.color, color_strings, 3);
		
		if(Combo("Level Curve", (int*)&g->set.highlight_along, highlight_strings, 4)) {
			if(g->set.highlight_along == axis::x) {
				c_3d_static.setAxis(glm::vec3(0,1,0));
			} else if(g->set.highlight_along == axis::y) {
				c_3d_static.setAxis(glm::vec3(1,0,0));	
			} else if(g->set.highlight_along == axis::z) {
				c_3d_static.setAxis(glm::vec3(0,0,1));
			}
		}
		if(g->set.highlight_along != axis::none) {
			Indent();
			Checkbox("Draw Graph", &g->set.highlight_draw_graph);
			Unindent();
		}
		c_3d_static.lock = g->set.highlight_along != axis::none && !g->set.highlight_draw_graph;

		changed = changed || Combo("Calculus", (int*)&g->set.calc, calc_strings, 7);

		SliderFloat("Opacity", &g->set.opacity, 0.0f, 1.0f);
		SliderFloat("Ambient Light", &g->set.ambientLighting, 0.0f, 1.0f);
	} else {
		changed = changed || Combo("Calculus", (int*)&g->set.calc, para_strings, 4);
	}

	switch(g->type) {
	case graph_func: {
		changed = changed || InputFloat("xmin", &g->set.rdom.xmin);
		changed = changed || InputFloat("xmax", &g->set.rdom.xmax);
		changed = changed || InputFloat("ymin", &g->set.rdom.ymin);
		changed = changed || InputFloat("ymax", &g->set.rdom.ymax);
		changed = changed || InputInt("xrez", &g->set.rdom.xrez);
		changed = changed || InputInt("yrez", &g->set.rdom.yrez);
	} break;
	case graph_cylindrical: {
		changed = changed || InputFloat("zmin", &g->set.cdom.zmin);
		changed = changed || InputFloat("zmax", &g->set.cdom.zmax);
		changed = changed || InputFloat("θmin", &g->set.cdom.tmin);
		changed = changed || InputFloat("θmax", &g->set.cdom.tmax);
		changed = changed || InputInt("zrez", &g->set.cdom.zrez);
		changed = changed || InputInt("θrez", &g->set.cdom.trez);
	} break;
	case graph_spherical: {
		changed = changed || InputFloat("θmin", &g->set.sdom.tmin);
		changed = changed || InputFloat("θmax", &g->set.sdom.tmax);
		changed = changed || InputFloat("φmin", &g->set.sdom.pmin);
		changed = changed || InputFloat("φmax", &g->set.sdom.pmax);
		changed = changed || InputInt("θrez", &g->set.sdom.trez);
		changed = changed || InputInt("φrez", &g->set.sdom.prez);
	} break;
	case graph_para_curve: {
		changed = changed || InputFloat("tmin", &g->set.pdom.tmin);
		changed = changed || InputFloat("tmax", &g->set.pdom.tmax);
		changed = changed || InputInt("trez", &g->set.pdom.trez);
	} break;
	}

	if(changed) {
		regengraph(ui.settings_index);
	}

	End();
}

void state::UIError() {
	using namespace ImGui;

	SetNextWindowPos({0.2f * w, 0.0f});
	Begin("Error!", &ui.error_shown, window_flags);
	Text(ui.error.c_str());
	if(Button("Dismiss")) {
		ui.error_shown = false;
	}
	End();
}

void state::UIHelp() {
	using namespace ImGui;

	SetNextWindowPos({0.2f * w, 0.0f});
	Begin("Help", &ui.help, window_flags);

	PushTextWrapPos(400);
	Separator();
	TextWrapped("Graphs");
	Indent();
	TextWrapped("Click the Add a Graph button to create a new function and enter its definition.");
	TextWrapped("To enter a Greek symbol, type the first letter of the symbol's English name and press tab. For example, to input θ, type 't' and press tab.");
	TextWrapped("Remember to include the * operator to denote multiplication.");
	TextWrapped("Press enter or click the ▶ button to render the graph. Click the ⚙ button to adjust domain and rendering settings.");
	Unindent();

	Separator();
	TextWrapped("Camera");
	Indent();
	TextWrapped("Click the Camera button to change the camera settings.");
	TextWrapped("The FOV (field of view) sets the zoom.");
	TextWrapped("The Free 3D camera allows you to fly around the scene in first person. Click and hold on the graph to navigate with W/A/S/D.");
	Unindent();
	PopTextWrapPos();
	End();

	SetNextWindowPos({0.2f * w + 400, 0.0f});
	Begin("Equation Features", nullptr, window_flags);
	PushTextWrapPos(400);
	Separator();
	Text("Operators");
	Indent();
	TextWrapped("+, -, *, /, %%, ^");
	Unindent();
	Separator();
	Text("Assignment");
	Indent();
	TextWrapped(":=, +=, -=, *=, /=, %%=");
	Unindent();
	Separator();
	Text("Comparison");
	Indent();
	TextWrapped("=, ==, <>, !=, <, <=, >, >=");
	Unindent();
	Separator();
	Text("Logic");
	Indent();
	TextWrapped("and, mand, mor, nand, nor, not, or, shl, shr, xnor, xor, true, false");
	Unindent();
	Separator();
	Text("Functions");
	Indent();
	TextWrapped("abs, avg, ceil, clamp, equal, erf, erfc,  exp, expm1, floor, frac, log, log10, log1p, log2, logn, max, min, mul, ncdf, nequal, root, round, roundn, sgn, sqrt, sum, swap, trunc");
	Unindent();
	Separator();
	Text("Trig");
	Indent();
	TextWrapped("acos, acosh, asin, asinh, atan, atanh, atan2, cos, cosh, cot, csc, sec, sin, sinc, sinh, tan, tanh, hypot, rad2deg, deg2grad, deg2rad, grad2deg");
	Unindent();
	Separator();
	Text("Control");
	Indent();
	TextWrapped("if-then-else, ternary conditional, switch-case, return-statement, while, for, repeat-until, break, continue");
	Unindent();
	PopTextWrapPos();
	End();
}

void state::UI() {
	using namespace ImGui;

	ImGui_ImplSdlGL3_NewFrame(window);

	SetNextWindowPos({0, 0});
	SetNextWindowSize({0.2f * w, (float)h});
	Begin("Main", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);

	if(Button("Add a Graph")) {
		ui.func = !ui.func;
	}
	SameLine();
	if(Button("Camera")) {
		ui.cam = !ui.cam;
	}
	SameLine();
	if(Button("Help")) {
		ui.help = !ui.help;
	}

	Separator();

	UIGraphs();

	if(ui.cam) {
		UICamera();
	}

	if(ui.func) {
		UIFunc();
	}

	if(ui.settings) {
		UISettings();
	}

	if(ui.error_shown) {
		UIError();
	}

	if(ui.help) {
		UIHelp();
	}

	End();
	glDisable(GL_POLYGON_SMOOTH);
	Render();
	glEnable(GL_POLYGON_SMOOTH);
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
			if(ui.current == mode::cam) {
				int dx = (e.motion.x - mx);
				int dy = (e.motion.y - my);
				if (camtype == cam_type::_3d) {
					c_3d.move(dx, dy);
				}
				else if (camtype == cam_type::_3d_static) {
					c_3d_static.move(dx, dy);
				}
			}
			mx = e.motion.x;
			my = e.motion.y;
		} break;

		case SDL_MOUSEBUTTONDOWN: {
			if(!io.WantCaptureMouse) {
				ui.current = mode::cam;
				SDL_CaptureMouse(SDL_TRUE);
				SDL_SetRelativeMouseMode(SDL_TRUE);
			}
			last_mx = e.button.x;
			last_my = e.button.y;
		} break;

		case SDL_MOUSEBUTTONUP: {
			if(!io.WantCaptureMouse && ui.current == mode::cam) {
				ui.current = mode::idle;
				SDL_CaptureMouse(SDL_FALSE);
				SDL_SetRelativeMouseMode(SDL_FALSE);
				SDL_WarpMouseInWindow(window, last_mx, last_my);
			}
		} break;

		case SDL_MOUSEWHEEL: {
			if(!io.WantCaptureMouse) {
				if(camtype == cam_type::_3d) {
					c_3d.fov -= 5 * e.wheel.y;
					if(c_3d.fov < 10) c_3d.fov = 10;
					if(c_3d.fov > 170) c_3d.fov = 170;
				} else if(camtype == cam_type::_3d_static) {
					c_3d_static.radius -=  e.wheel.y / 2.5f;
					if(c_3d_static.radius < 0.5f) c_3d_static.radius = 0.5f;
					c_3d_static.updatePos();
				}
			}
		} break;
		}
	}

	float dT = (SDL_GetTicks() - c_3d.lastUpdate) / 1000.0f;
	c_3d.lastUpdate = SDL_GetTicks();
	if (camtype == cam_type::_3d && ui.current == mode::cam) {
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
