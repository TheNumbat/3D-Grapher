
#define CONST_E  2.71828182845f
#define CONST_PI 3.14159265359f

graph::graph(int id) {
	eq_str.resize(1000, 0);
	ID = id;
	xmin = xmax = ymin = ymax = zmin = zmax = 0;
	gen();
}

graph::~graph() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &normVBO);
}

void graph::gen() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenBuffers(1, &normVBO);
}

void graph::send() {
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verticies.size(), verticies.size() ? &verticies[0] : NULL, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, normVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), normals.size() ? &normals[0] : NULL, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indicies.size(), indicies.size() ? &indicies[0] : NULL, GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void graph::draw(state* s, glm::mat4 model, glm::mat4 view, glm::mat4 proj) {
	if(!verticies.size()) return;

	glBindVertexArray(VAO);
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);

		glm::mat4 modelviewproj = proj * view * model;

		if (set.lighting) {
			if(set.normal_colors) {
				s->graph_s_norm.use();
			} else {
				s->graph_s_light.use();
			}

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, normVBO);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(1);

			glUniformMatrix4fv(s->graph_s_light.getUniform("model"), 1, GL_FALSE, value_ptr(model));
			glUniformMatrix4fv(s->graph_s_light.getUniform("modelviewproj"), 1, GL_FALSE, value_ptr(modelviewproj));

			glUniform4f(s->graph_s_light.getUniform("vcolor"), 0.8f, 0.8f, 0.8f, set.opacity);
			glUniform3f(s->graph_s_light.getUniform("lightColor"), 1.0f, 1.0f, 1.0f);
			glUniform1f(s->graph_s_light.getUniform("ambientStrength"), set.ambientLighting);

			if (s->camtype == cam_type::_3d) {
				glUniform3f(s->graph_s_light.getUniform("lightPos"), s->c_3d.pos.x, s->c_3d.pos.y, s->c_3d.pos.z);
			} else {
				glUniform3f(s->graph_s_light.getUniform("lightPos"), s->c_3d_static.pos.x, s->c_3d_static.pos.y, s->c_3d_static.pos.z);
			}
		} else {
			s->graph_s.use();

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);

			glUniformMatrix4fv(s->graph_s.getUniform("modelviewproj"), 1, GL_FALSE, value_ptr(modelviewproj));

			glUniform4f(s->graph_s.getUniform("vcolor"), 0.8f, 0.8f, 0.8f, set.opacity);
		}

		if (set.wireframe) {
			glDrawElements(GL_LINES, (int)indicies.size(), GL_UNSIGNED_INT, (void*)0);
		} else {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glPolygonOffset(1.0f, 0.0f);
			glDrawElements(GL_TRIANGLES, (int)indicies.size(), GL_UNSIGNED_INT, (void*)0);
		}

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}
	glBindVertexArray(0);
}

void graph::normalize() {
	if (set.axisnormalization) {
		for (unsigned int i = 0; i < verticies.size(); i += 3) {
			verticies[i] /= (set.rdom.xmax - set.rdom.xmin) / 20;
			verticies[i + 1] /= (set.rdom.ymax - set.rdom.ymin) / 20;
			verticies[i + 2] /= (zmax - zmin) / 20;
		}
		zmin = -10;
		zmax = 10;
	}
}

void graph::generateIndiciesAndNormals() {
	indicies.clear();
	normals.clear();

	glm::vec3 norm;
	int _x_max = 0;
	int _y_max = 0;
	if (type == graph_func) {
		_x_max = set.rdom.xrez;
		_y_max = set.rdom.yrez;
	}
	else if (type == graph_cylindrical) {
		_x_max = set.cdom.trez;
		_y_max = set.cdom.zrez;
	}
	else if (type == graph_spherical) {
		_x_max = set.sdom.prez;
		_y_max = set.sdom.trez;
	}

	for (int x = 0; x < _x_max; x++) {
		for (int y = 0; y < _y_max; y++) {
			GLuint i_index = x * (_y_max + 1) + y;

			if (!isnan(verticies[i_index * 3 + 2]) &&
				!isinf(verticies[i_index * 3 + 2])) {
				indicies.push_back(i_index);
				indicies.push_back(i_index + 1);
				indicies.push_back(i_index + _y_max + 1);

				indicies.push_back(i_index + 1);
				indicies.push_back(i_index + _y_max + 1);
				indicies.push_back(i_index + _y_max + 2);

				float x1 = verticies[i_index * 3];
				float y1 = verticies[i_index * 3 + 1];
				float z1 = verticies[i_index * 3 + 2];
				float x2 = verticies[(i_index + 1) * 3];
				float y2 = verticies[(i_index + 1) * 3 + 1];
				float z2 = verticies[(i_index + 1) * 3 + 2];
				float x3 = verticies[(i_index + _y_max + 1) * 3];
				float y3 = verticies[(i_index + _y_max + 1) * 3 + 1];
				float z3 = verticies[(i_index + _y_max + 1) * 3 + 2];
				glm::vec3 one(x2 - x1, y2 - y1, z2 - z1);
				glm::vec3 two(x3 - x1, y3 - y1, z3 - z1);
				norm = glm::normalize(cross(two, one));
			}
			normals.push_back(norm);
			if (x == 0)
				normals.push_back(norm);
		}
		normals.push_back(norm);
	}
	normals.push_back(norm);
}

fxy_graph::fxy_graph(int id) : graph(id) {
	type = graph_func;
	set.rdom = { -10, 10, -10, 10, 200, 200 };
}

void fxy_graph::genthread(gendata* g) {

	double x = g->xmin;
	double y = g->dom.ymin;

	exprtk::expression<double> expr;
	exprtk::symbol_table<double> table;
	exprtk::parser<double> parser;
	table.add_constant(L"e", CONST_E);
	std::string pi_utf("π");
	table.add_constant(utf8_to_wstring(pi_utf),CONST_PI);
	table.add_constants();
	table.add_variable(L"x",x);
	table.add_variable(L"y",y);
	expr.register_symbol_table(table);

	if(!parser.compile(utf8_to_wstring(trim_end(g->eq)), expr)) {
		std::lock_guard<std::mutex> lock(g->s->ui.error_mut);
		g->s->ui.error_shown = true;
		g->s->ui.error = wstring_to_utf8(parser.error());
		g->success = false;
		return;
	}

	for (int tx = 0; tx < g->txrez; tx++, x += g->dx) {
		y = g->dom.ymin;
		for (int ty = 0; ty <= g->dom.yrez; ty++, y += g->dy) {
			double z = 0.0f;

			switch(g->calc) {
			case calculus::none: 	z = expr.value(); break;
			case calculus::part_x:	z = exprtk::derivative(expr, x, 0.00001); break;
			case calculus::part_y:	z = exprtk::derivative(expr, y, 0.00001); break;
			case calculus::part2_x:	z = exprtk::second_derivative(expr, x, 0.0001); break;
			case calculus::part2_y:	z = exprtk::second_derivative(expr, y, 0.0001); break;
			case calculus::part3_x:	z = exprtk::third_derivative(expr, x, 0.001); break;
			case calculus::part3_y:	z = exprtk::third_derivative(expr, y, 0.001); break;
			}

			if (z < g->zmin) g->zmin = (float)z;
			if (z > g->zmax) g->zmax = (float)z;

			g->ret.push_back((float)x);
			g->ret.push_back((float)y);
			g->ret.push_back((float)z);
		}
	}
	g->success = true;
}

void fxy_graph::generate(state* s) {
	unsigned int numthreads = std::thread::hardware_concurrency();
#ifdef _MSC_VER
	int cpuinfo[4];
	__cpuid(cpuinfo, 1);
	bool HT = (cpuinfo[3] & (1 << 28)) > 0;
	if (HT) numthreads /= 2;
#endif

	xmin = ymin = zmin = FLT_MAX;
	xmax = ymax = zmax = -FLT_MAX;

	float dx = (set.rdom.xmax - set.rdom.xmin) / set.rdom.xrez;
	float dy = (set.rdom.ymax - set.rdom.ymin) / set.rdom.yrez;
	xmin = set.rdom.xmin;
	ymin = set.rdom.ymin;
	xmax = set.rdom.xmax;
	ymax = set.rdom.ymax;
	float _xmin = xmin;
	unsigned int txDelta = set.rdom.xrez / numthreads;
	unsigned int txLast = set.rdom.xrez - (numthreads - 1) * txDelta + 1;

	verticies.clear();

	std::vector<std::thread> threads;
	std::vector<gendata*> data;
	for (unsigned int i = 0; i < numthreads; i++) {
		if (txDelta || i == numthreads - 1) {
			gendata* d = new gendata;

			if (i == numthreads - 1)
				d->txrez = txLast;
			else
				d->txrez = txDelta;

			d->s = s;
			d->dom = set.rdom;
			d->eq = eq_str;
			d->dx = dx;
			d->dy = dy;
			d->xmin = _xmin;
			d->ID = ID;
			d->calc = set.calc;

			data.push_back(d);
			threads.push_back(std::thread(genthread, data.back()));

			_xmin += txDelta * dx;
		}
	}
	bool success = true;
	for (unsigned int i = 0; i < threads.size(); i++) {
		threads[i].join();
		if (success) {
			success = data[i]->success;
			verticies.insert(verticies.end(), data[i]->ret.begin(), data[i]->ret.end());
			data[i]->ret.clear();
		}
	}

	if (success) {
		float gzmin = FLT_MAX, gzmax = -FLT_MAX;
		for (unsigned int i = 0; i < threads.size(); i++) {
			if (data[i]->zmin < gzmin) gzmin = data[i]->zmin;
			if (data[i]->zmax > gzmax) gzmax = data[i]->zmax;
		}
		zmin = gzmin;
		zmax = gzmax;

		normalize();
		generateIndiciesAndNormals();
	}

	for (gendata* g : data)
		delete g;
}

cyl_graph::cyl_graph(int id) : graph(id) {
	type = graph_cylindrical;
	set.cdom = { 0, 1, 0, 2 * (float)CONST_PI, 200, 200 };
}

void cyl_graph::genthread(gendata* g) {
	
	double z = g->zmin;
	double t = g->dom.tmin;

	exprtk::expression<double> expr;
	exprtk::symbol_table<double> table;
	exprtk::parser<double> parser;
	table.add_constant(L"e", CONST_E);
	std::string pi_utf("π");
	table.add_constant(utf8_to_wstring(pi_utf),CONST_PI);
	table.add_constants();
	table.add_variable(L"z",z);
	std::string theta_utf("θ");
	table.add_variable(utf8_to_wstring(theta_utf),t);

	expr.register_symbol_table(table);

	if(!parser.compile(utf8_to_wstring(trim_end(g->eq)), expr)) {
		std::lock_guard<std::mutex> lock(g->s->ui.error_mut);
		g->s->ui.error_shown = true;
		g->s->ui.error = wstring_to_utf8(parser.error());
		g->success = false;
		return;
	}

	for (int tz = 0; tz < g->tzrez; tz++, z += g->dz) {
		t = g->dom.tmin;
		for (int tt = 0; tt <= g->dom.trez; tt++, t += g->dt) {
			double r = 0.0f;

			switch(g->calc) {
			case calculus::none: 	r = expr.value(); break;
			case calculus::part_x:	r = exprtk::derivative(expr, z, 0.00001); break;
			case calculus::part_y:	r = exprtk::derivative(expr, t, 0.00001); break;
			case calculus::part2_x:	r = exprtk::second_derivative(expr, z, 0.0001); break;
			case calculus::part2_y:	r = exprtk::second_derivative(expr, t, 0.0001); break;
			case calculus::part3_x:	r = exprtk::third_derivative(expr, z, 0.001); break;
			case calculus::part3_y:	r = exprtk::third_derivative(expr, t, 0.001); break;
			}

			double x = r * cos(t);
			double y = r * sin(t);
			if (x < g->gxmin) g->gxmin = (float)x;
			if (x > g->gxmax) g->gxmax = (float)x;
			if (y < g->gymin) g->gymin = (float)y;
			if (y > g->gymax) g->gymax = (float)y;

			g->ret.push_back((float)x);
			g->ret.push_back((float)y);
			g->ret.push_back((float)z);
		}
	}
	g->success = true;
}

void cyl_graph::generate(state* s) {
	unsigned int numthreads = std::thread::hardware_concurrency();
#ifdef _MSC_VER
	int cpuinfo[4];
	__cpuid(cpuinfo, 1);
	bool HT = (cpuinfo[3] & (1 << 28)) > 0;
	if (HT) numthreads /= 2;
#endif

	xmin = ymin = zmin = FLT_MAX;
	xmax = ymax = zmax = -FLT_MAX;

	float dz = (set.cdom.zmax - set.cdom.zmin) / set.cdom.zrez;
	float dt = (set.cdom.tmax - set.cdom.tmin) / set.cdom.trez;
	zmin = set.cdom.zmin;
	zmax = set.cdom.zmax;
	float _zmin = zmin;
	unsigned int tzDelta = set.cdom.zrez / numthreads;
	unsigned int tzLast = set.cdom.zrez - (numthreads - 1) * tzDelta + 1;

	verticies.clear();

	std::vector<std::thread> threads;
	std::vector<gendata*> data;
	for (unsigned int i = 0; i < numthreads; i++) {
		if (tzDelta || i == numthreads - 1) {
			gendata* d = new gendata;

			if (i == numthreads - 1)
				d->tzrez = tzLast;
			else
				d->tzrez = tzDelta;

			d->s = s;
			d->eq = eq_str;
			d->dom = set.cdom;
			d->dz = dz;
			d->dt = dt;
			d->zmin = _zmin;
			d->ID = ID;
			d->calc = set.calc;

			data.push_back(d);
			threads.push_back(std::thread(genthread, data.back()));

			_zmin += tzDelta * dz;
		}
	}
	bool success = true;
	for (unsigned int i = 0; i < threads.size(); i++) {
		threads[i].join();
		if (success) {
			success = data[i]->success;
			verticies.insert(verticies.end(), data[i]->ret.begin(), data[i]->ret.end());
			data[i]->ret.clear();
		}
	}

	if (success) {
		float gxmin = FLT_MAX, gxmax = -FLT_MAX;
		for (unsigned int i = 0; i < threads.size(); i++) {
			if (data[i]->gxmin < gxmin) gxmin = data[i]->gxmin;
			if (data[i]->gxmax > gxmax) gxmax = data[i]->gxmax;
		}
		xmin = gxmin;
		xmax = gxmax;
		float gymin = FLT_MAX, gymax = -FLT_MAX;
		for (unsigned int i = 0; i < threads.size(); i++) {
			if (data[i]->gymin < gymin) gymin = data[i]->gymin;
			if (data[i]->gymax > gymax) gymax = data[i]->gymax;
		}
		ymin = gymin;
		ymax = gymax;

		normalize();
		generateIndiciesAndNormals();
	}

	for (gendata* g : data)
		delete g;
}

spr_graph::spr_graph(int id) : graph(id) {
	type = graph_spherical;
	set.sdom = { 0, 2 * (float)CONST_PI, 0, (float)CONST_PI, 200, 200 };
}

void spr_graph::genthread(gendata* g) {

	double p = g->pmin;
	double t = g->dom.tmin;

	exprtk::expression<double> expr;
	exprtk::symbol_table<double> table;
	exprtk::parser<double> parser;
	table.add_constant(L"e", CONST_E);
	std::string pi_utf("π");
	table.add_constant(utf8_to_wstring(pi_utf),CONST_PI);
	table.add_constants();
	std::string phi_utf("φ");
	table.add_variable(utf8_to_wstring(phi_utf),p);
	std::string theta_utf("θ");
	table.add_variable(utf8_to_wstring(theta_utf),t);
	expr.register_symbol_table(table);

	if(!parser.compile(utf8_to_wstring(trim_end(g->eq)), expr)) {
		std::lock_guard<std::mutex> lock(g->s->ui.error_mut);
		g->s->ui.error_shown = true;
		g->s->ui.error = wstring_to_utf8(parser.error());
		g->success = false;
		return;
	}

	for (int tp = 0; tp < g->tprez; tp++, p += g->dp) {
		t = g->dom.tmin;
		for (int tt = 0; tt <= g->dom.trez; tt++, t += g->dt) {
			double r = 0.0f;

			switch(g->calc) {
			case calculus::none: 	r = expr.value(); break;
			case calculus::part_x:	r = exprtk::derivative(expr, p, 0.00001); break;
			case calculus::part_y:	r = exprtk::derivative(expr, t, 0.00001); break;
			case calculus::part2_x:	r = exprtk::second_derivative(expr, p, 0.0001); break;
			case calculus::part2_y:	r = exprtk::second_derivative(expr, t, 0.0001); break;
			case calculus::part3_x:	r = exprtk::third_derivative(expr, p, 0.001); break;
			case calculus::part3_y:	r = exprtk::third_derivative(expr, t, 0.001); break;
			}			

			double x = r * cos(t) * sin(p);
			double y = r * sin(t) * sin(p);
			double z = r * cos(p);

			if (z < g->gzmin) g->gzmin = (float)z;
			if (z > g->gzmax) g->gzmax = (float)z;
			if (y < g->gymin) g->gymin = (float)y;
			if (y > g->gymax) g->gymax = (float)y;
			if (x < g->gxmin) g->gxmin = (float)x;
			if (x > g->gxmax) g->gxmax = (float)x;

			g->ret.push_back((float)x);
			g->ret.push_back((float)y);
			g->ret.push_back((float)z);
		}
	}
	g->success = true;
}

void spr_graph::generate(state* s) {
	unsigned int numthreads = std::thread::hardware_concurrency();
#ifdef _MSC_VER
	int cpuinfo[4];
	__cpuid(cpuinfo, 1);
	bool HT = (cpuinfo[3] & (1 << 28)) > 0;
	if (HT) numthreads /= 2;
#endif

	xmin = ymin = zmin = FLT_MAX;
	xmax = ymax = zmax = -FLT_MAX;

	float dt = (set.sdom.tmax - set.sdom.tmin) / set.sdom.trez;
	float dp = (set.sdom.pmax - set.sdom.pmin) / set.sdom.prez;
	float pmin = set.sdom.pmin;
	unsigned int tpDelta = set.sdom.prez / numthreads;
	unsigned int tpLast = set.sdom.prez - (numthreads - 1) * tpDelta + 1;

	verticies.clear();

	std::vector<std::thread> threads;
	std::vector<gendata*> data;
	for (unsigned int i = 0; i < numthreads; i++) {
		if (tpDelta || i == numthreads - 1) {
			gendata* d = new gendata;

			if (i == numthreads - 1)
				d->tprez = tpLast;
			else
				d->tprez = tpDelta;

			d->s = s;
			d->dom = set.sdom;
			d->eq = eq_str;
			d->dt = dt;
			d->dp = dp;
			d->pmin = pmin;
			d->ID = ID;
			d->calc = set.calc;

			data.push_back(d);
			threads.push_back(std::thread(genthread, data.back()));

			pmin += tpDelta * dp;
		}
	}
	bool success = true;
	for (unsigned int i = 0; i < threads.size(); i++) {
		threads[i].join();
		if (success) {
			success = data[i]->success;
			verticies.insert(verticies.end(), data[i]->ret.begin(), data[i]->ret.end());
			data[i]->ret.clear();
		}
	}

	if (success) {
		float gzmin = FLT_MAX, gzmax = -FLT_MAX;
		for (unsigned int i = 0; i < threads.size(); i++) {
			if (data[i]->gzmin < gzmin) gzmin = data[i]->gzmin;
			if (data[i]->gzmax > gzmax) gzmax = data[i]->gzmax;
		}
		zmin = gzmin;
		zmax = gzmax;
		float gxmin = FLT_MAX, gxmax = -FLT_MAX;
		for (unsigned int i = 0; i < threads.size(); i++) {
			if (data[i]->gxmin < gxmin) gxmin = data[i]->gxmin;
			if (data[i]->gxmax > gxmax) gxmax = data[i]->gxmax;
		}
		xmin = gxmin;
		xmax = gxmax;
		float gymin = FLT_MAX, gymax = -FLT_MAX;
		for (unsigned int i = 0; i < threads.size(); i++) {
			if (data[i]->gymin < gymin) gymin = data[i]->gymin;
			if (data[i]->gymax > gymax) gymax = data[i]->gymax;
		}
		ymin = gymin;
		ymax = gymax;

		normalize();
		generateIndiciesAndNormals();
	}

	for (gendata* g : data)
		delete g;
}

para_curve::para_curve(int id) : graph(id) {
	type = graph_para_curve;
	eqx.resize(1000, 0);
	eqy.resize(1000, 0);
	eqz.resize(1000, 0);
	set.pdom = { 0, 10, 100 };
}

void para_curve::generate(state* s) {

	xmin = ymin = zmin = FLT_MAX;
	xmax = ymax = zmax = -FLT_MAX;

	verticies.clear();
	double t = set.pdom.tmin;

	exprtk::expression<double> expr_x, expr_y, expr_z;
	exprtk::symbol_table<double> table;
	exprtk::parser<double> parser;
	table.add_constant(L"e", CONST_E);
	std::string pi_utf("π");
	table.add_constant(utf8_to_wstring(pi_utf),CONST_PI);
	table.add_constants();
	table.add_variable(L"t",t);
	expr_x.register_symbol_table(table);
	expr_y.register_symbol_table(table);
	expr_z.register_symbol_table(table);

	if(!parser.compile(utf8_to_wstring(trim_end(eqx)), expr_x)) {
		std::lock_guard<std::mutex> lock(s->ui.error_mut);
		s->ui.error_shown = true;
		s->ui.error = wstring_to_utf8(parser.error());
		return;
	}
	if(!parser.compile(utf8_to_wstring(trim_end(eqy)), expr_y)) {
		std::lock_guard<std::mutex> lock(s->ui.error_mut);
		s->ui.error_shown = true;
		s->ui.error = wstring_to_utf8(parser.error());
		return;
	}
	if(!parser.compile(utf8_to_wstring(trim_end(eqz)), expr_z)) {
		std::lock_guard<std::mutex> lock(s->ui.error_mut);
		s->ui.error_shown = true;
		s->ui.error = wstring_to_utf8(parser.error());
		return;
	}

	for (int tstep = 0; tstep < set.pdom.trez; tstep++, t += (set.pdom.tmax - set.pdom.tmin) / set.pdom.trez) {
		double x = 0.0f;
		switch(set.calc) {
		case calculus::none: 	x = expr_x.value(); break;
		case calculus::part_x:	x = exprtk::derivative(expr_x, t, 0.00001); break;
		case calculus::part_y:	x = exprtk::second_derivative(expr_x, t, 0.0001); break;
		case calculus::part2_x:	x = exprtk::third_derivative(expr_x, t, 0.001); break;
		}
		double y = 0.0f;
		switch(set.calc) {
		case calculus::none: 	y = expr_y.value(); break;
		case calculus::part_x:	y = exprtk::derivative(expr_y, t, 0.00001); break;
		case calculus::part_y:	y = exprtk::second_derivative(expr_y, t, 0.0001); break;
		case calculus::part2_x:	y = exprtk::third_derivative(expr_y, t, 0.001); break;
		}
		double z = 0.0f;
		switch(set.calc) {
		case calculus::none: 	z = expr_z.value(); break;
		case calculus::part_x:	z = exprtk::derivative(expr_z, t, 0.00001); break;
		case calculus::part_y:	z = exprtk::second_derivative(expr_z, t, 0.0001); break;
		case calculus::part2_x:	z = exprtk::third_derivative(expr_z, t, 0.001); break;
		}		

		if (z < zmin) zmin = (float)z;
		if (z > zmax) zmax = (float)z;
		if (y < ymin) ymin = (float)y;
		if (y > ymax) ymax = (float)y;
		if (x < xmin) xmin = (float)x;
		if (x > xmax) xmax = (float)x;

		verticies.push_back((float)x);
		verticies.push_back((float)y);
		verticies.push_back((float)z);
	}
	generateIndiciesAndNormals();
}

void para_curve::generateIndiciesAndNormals() {
	indicies.clear();
	indicies.push_back(0);
	for (int i = 1; i < set.pdom.trez - 1; i++) {
		indicies.push_back(i);
		indicies.push_back(i);
	}
	indicies.push_back(set.pdom.trez - 1);
}

void para_curve::draw(state* s, glm::mat4 model, glm::mat4 view, glm::mat4 proj) {
	glBindVertexArray(VAO);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	glm::mat4 modelviewproj = proj * view * model;

	s->graph_s.use();

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glUniformMatrix4fv(s->graph_s.getUniform("modelviewproj"), 1, GL_FALSE, value_ptr(modelviewproj));
	glUniform4f(s->graph_s.getUniform("vcolor"), 0.0f, 0.0f, 0.0f, set.opacity);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPolygonOffset(1.0f, 0.0f);
	glDrawElements(GL_LINES, (int)indicies.size(), GL_UNSIGNED_INT, (void*)0);

	glBindVertexArray(0);
}
