
#define CONST_E  2.71828182845f
#define CONST_PI 3.14159265359f

double select_calc(calculus calc, const exprtk::expression<double>& expr, double& t) {
	switch(calc) {
	case calculus::none: 	return expr.value();
	case calculus::part_x:	return exprtk::derivative(expr, t, 0.00001);
	case calculus::part_y:	return exprtk::second_derivative(expr, t, 0.0001);
	case calculus::part2_x:	return exprtk::third_derivative(expr, t, 0.001);
	default: return 0.0;
	}
}

double select_calc(calculus calc, const exprtk::expression<double>& expr, double& x, double& y) {
	switch(calc) {
	case calculus::none: 	return expr.value();
	case calculus::part_x:	return exprtk::derivative(expr, x, 0.00001);
	case calculus::part_y:	return exprtk::derivative(expr, y, 0.00001);
	case calculus::part2_x:	return exprtk::second_derivative(expr, x, 0.0001);
	case calculus::part2_y:	return exprtk::second_derivative(expr, y, 0.0001);
	case calculus::part3_x:	return exprtk::third_derivative(expr, x, 0.001);
	case calculus::part3_y:	return exprtk::third_derivative(expr, y, 0.001);
	default: return 0.0;
	}
}

glm::vec2 get_grad(const exprtk::expression<double>& expr, double& x, double& y) {

	glm::vec2 ret;
	ret.x = (float)exprtk::derivative(expr, x, 0.00001);
	ret.y = (float)exprtk::derivative(expr, y, 0.00001);
	return ret;
}

exprtk::expression<double> make_expression(std::string eq_utf, exprtk::symbol_table<double>& table, state::uistate& ui) {
	
	exprtk::expression<double> expr;
	exprtk::parser<double> parser;
	expr.register_symbol_table(table);

	if(!parser.compile(utf8_to_wstring(trim_end(eq_utf)), expr)) {
		std::lock_guard<std::mutex> lock(ui.error_mut);
		ui.error_shown = true;
		ui.error = wstring_to_utf8(parser.error());
	}

	return expr;
}

exprtk::symbol_table<double> default_table() {

	exprtk::symbol_table<double> table;
	
	std::string pi_utf("π");

	table.add_constant(utf8_to_wstring(pi_utf),CONST_PI);
	table.add_constant(L"e", CONST_E);

	table.add_constants();
	
	return table;
}

bool isnan(glm::vec3 v) {
	return std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z);
}

bool isinf(glm::vec3 v) {
	return std::isinf(v.x) || std::isinf(v.y) || std::isinf(v.z);
}

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
	glDeleteBuffers(1, &colorVBO);
}

void graph::gen() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenBuffers(1, &normVBO);
	glGenBuffers(1, &colorVBO);
}

void graph::clear() {
	indicies.clear();
	normals.clear();
	colors.clear();
	verticies.clear();

	xmin = ymin = zmin = FLT_MAX;
	xmax = ymax = zmax = -FLT_MAX;
}

void graph::send() {
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * verticies.size(), verticies.size() ? &verticies[0] : nullptr, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, normVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), normals.size() ? &normals[0] : nullptr, GL_STATIC_DRAW);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * colors.size(), colors.size() ? &colors[0] : nullptr, GL_STATIC_DRAW);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indicies.size(), indicies.size() ? &indicies[0] : nullptr, GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void graph::clampInfBounds() {
	if(std::isnan(xmin) || std::isinf(xmin)) xmin = -100;
	if(std::isnan(xmax) || std::isinf(xmax)) xmax =  100;
	if(std::isnan(ymin) || std::isinf(ymin)) ymin = -100;
	if(std::isnan(ymax) || std::isinf(ymax)) ymax =  100;
	if(std::isnan(zmin) || std::isinf(zmin)) zmin = -100;
	if(std::isnan(zmax) || std::isinf(zmax)) zmax =  100;
}

void graph::draw(state* s, glm::mat4 model, glm::mat4 view, glm::mat4 proj) {
	if(!verticies.size()) return;

	glBindVertexArray(VAO);
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);

		glm::mat4 modelviewproj = proj * view * model;

		s->graph_s.use();

		glUniformMatrix4fv(s->graph_s.getUniform("model"), 1, GL_FALSE, value_ptr(model));
		glUniformMatrix4fv(s->graph_s.getUniform("modelviewproj"), 1, GL_FALSE, value_ptr(modelviewproj));

		glUniform3f(s->graph_s.getUniform("lightColor"), 1.0f, 1.0f, 1.0f);
		glUniform1f(s->graph_s.getUniform("ambientStrength"), set.ambientLighting);
		glUniform1f(s->graph_s.getUniform("opacity"), set.opacity);

		glUniform1i(s->graph_s.getUniform("lighting"), set.lighting);
		glUniform1i(s->graph_s.getUniform("highlight"), (int)set.highlight_along);
		glUniform3fv(s->graph_s.getUniform("highlight_pos"), 1, value_ptr(set.highlight_value));
		glUniform1f(s->graph_s.getUniform("tolerance"), 1.0f);

		if (s->camtype == cam_type::_3d) {
			glUniform3f(s->graph_s.getUniform("lightPos"), s->c_3d.pos.x, s->c_3d.pos.y, s->c_3d.pos.z);
		} else {
			glUniform3f(s->graph_s.getUniform("lightPos"), s->c_3d_static.pos.x, s->c_3d_static.pos.y, s->c_3d_static.pos.z);
		}

		if (set.wireframe) {
			glDrawElements(GL_LINES, (int)indicies.size(), GL_UNSIGNED_INT, (void*)0);
		} else {
			glDrawElements(GL_TRIANGLES, (int)indicies.size(), GL_UNSIGNED_INT, (void*)0);
		}
	}
	glBindVertexArray(0);
}

void graph::normalize() {
	if (set.axisnormalization) {
		for (unsigned int i = 0; i < verticies.size(); i += 3) {
			verticies[i] 	 /= (xmax - xmin) / 20;
			verticies[i + 1] /= (ymax - ymin) / 20;
			verticies[i + 2] /= (zmax - zmin) / 20;
		}
		zmin = -10;
		zmax = 10;
	}
}

void graph::generateIndiciesAndNormals() {

	glm::vec3 norm;
	int _x_max = 0;
	int _y_max = 0;
	if (type == graph_func) {
		_x_max = set.rdom.xrez;
		_y_max = set.rdom.yrez;
	}
	else if (type == graph_cylindrical) {
		_x_max = set.cdom.zrez;
		_y_max = set.cdom.trez;
	}
	else if (type == graph_spherical) {
		_x_max = set.sdom.prez;
		_y_max = set.sdom.trez;
	}

	for (int x = 0; x < _x_max; x++) {
		for (int y = 0; y < _y_max; y++) {
			GLuint i_index = x * (_y_max + 1) + y;

			if (!isnan(verticies[i_index + 2]) &&
				!isinf(verticies[i_index + 2])) {
				indicies.push_back(i_index);
				indicies.push_back(i_index + 1);
				indicies.push_back(i_index + _y_max + 1);

				indicies.push_back(i_index + 1);
				indicies.push_back(i_index + _y_max + 1);
				indicies.push_back(i_index + _y_max + 2);

				glm::vec3 p1 = verticies[i_index];
				glm::vec3 p2 = verticies[i_index + 1];
				glm::vec3 p3 = verticies[i_index + _y_max + 1];
				
				glm::vec3 one = p2 - p1;
				glm::vec3 two = p3 - p1;
				norm = glm::normalize(cross(one, two));
			}
			normals.push_back(norm);
			if (x == 0)
				normals.push_back(norm);
		}
		normals.push_back(norm);
	}
	normals.push_back(norm);

	switch(set.color) {
	case color_by::nothing: {
		for(int i = 0; i < normals.size(); i++) {
			colors.push_back(glm::vec3(0.8, 0.8, 0.8));
		}
	} break;
	case color_by::normal: {
		for(glm::vec3& n : normals) {
			colors.push_back(glm::abs(n));
		}
	} break;
	}
}

fxy_graph::fxy_graph(int id) : graph(id) {
	type = graph_func;
	set.rdom = { -10, 10, -10, 10, 200, 200 };
}

void fxy_graph::genthread(gendata* d) {

	double x = d->xmin;
	double y = d->set.rdom.ymin;

	exprtk::symbol_table<double> table = default_table();
	table.add_variable(L"x",x);
	table.add_variable(L"y",y);

	exprtk::expression<double> expr = make_expression(d->eq, table, d->s->ui);
	if(d->s->ui.error_shown) {
		d->success = false;
		return;
	}

	for (int tx = 0; tx < d->txrez; tx++, x += d->dx) {
		y = d->set.rdom.ymin;
		for (int ty = 0; ty <= d->set.rdom.yrez; ty++, y += d->dy) {
			
			double z = select_calc(d->set.calc, expr, x, y);
			if(d->set.color == color_by::gradient) {
				glm::vec2 xy = get_grad(expr, x, y);
				d->grad.push_back(glm::abs(glm::vec3(xy, 0)));
			}

			d->zmin = std::min(d->zmin, (float)z);
			d->zmax = std::max(d->zmax, (float)z);
			d->func.push_back(glm::vec3(x, y, z));
		}
	}
	d->success = true;
}

void fxy_graph::generate(state* s) {
	unsigned int numthreads = std::thread::hardware_concurrency();
#ifdef _MSC_VER
	int cpuinfo[4];
	__cpuid(cpuinfo, 1);
	bool HT = (cpuinfo[3] & (1 << 28)) > 0;
	if (HT) numthreads /= 2;
#endif

	clear();

	float dx = (set.rdom.xmax - set.rdom.xmin) / set.rdom.xrez;
	float dy = (set.rdom.ymax - set.rdom.ymin) / set.rdom.yrez;
	xmin = set.rdom.xmin;
	ymin = set.rdom.ymin;
	xmax = set.rdom.xmax;
	ymax = set.rdom.ymax;
	float _xmin = xmin;
	unsigned int txDelta = set.rdom.xrez / numthreads;
	unsigned int txLast = set.rdom.xrez - (numthreads - 1) * txDelta + 1;

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
			d->set = set;
			d->eq = eq_str;
			d->dx = dx;
			d->dy = dy;
			d->xmin = _xmin;
			d->ID = ID;

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
			verticies.insert(verticies.end(), data[i]->func.begin(), data[i]->func.end());
			data[i]->func.clear();

			if(set.color == color_by::gradient) {
				colors.insert(colors.end(), data[i]->grad.begin(), data[i]->grad.end());
				data[i]->grad.clear();
			}
		}
	}

	if (success) {
		zmin = FLT_MAX; zmax = -FLT_MAX;
		for (unsigned int i = 0; i < threads.size(); i++) {
			zmin = std::min(data[i]->zmin,zmin);
			zmax = std::max(data[i]->zmax,zmax);
		}

		normalize();
		clampInfBounds();
		generateIndiciesAndNormals();
	}

	for (gendata* g : data)
		delete g;
}

cyl_graph::cyl_graph(int id) : graph(id) {
	type = graph_cylindrical;
	set.cdom = { 0, 1, 0, 2 * (float)CONST_PI, 200, 200 };
}

void cyl_graph::genthread(gendata* d) {
	
	double z = d->zmin;
	double t = d->set.cdom.tmin;

	exprtk::symbol_table<double> table = default_table();
	std::string theta_utf("θ");

	table.add_variable(L"z",z);
	table.add_variable(utf8_to_wstring(theta_utf),t);

	exprtk::expression<double> expr = make_expression(d->eq, table, d->s->ui);
	if(d->s->ui.error_shown) {
		d->success = false;
		return;
	}

	for (int tz = 0; tz < d->tzrez; tz++, z += d->dz) {
		t = d->set.cdom.tmin;
		for (int tt = 0; tt <= d->set.cdom.trez; tt++, t += d->dt) {
			double r = select_calc(d->set.calc, expr, z, t);

			if(d->set.color == color_by::gradient) {
				glm::vec2 xy = get_grad(expr, z, t);
				d->grad.push_back(glm::abs(glm::vec3(xy, 0)));
			}

			double x = r * cos(t);
			double y = r * sin(t);
			d->gxmin = std::min(d->gxmin, (float)x);
			d->gxmax = std::max(d->gxmax, (float)x);
			d->gymin = std::min(d->gymin, (float)y);
			d->gymax = std::max(d->gymax, (float)y);
			d->func.push_back(glm::vec3(x, y, z));
		}
	}
	d->success = true;
}

void cyl_graph::generate(state* s) {
	unsigned int numthreads = std::thread::hardware_concurrency();
#ifdef _MSC_VER
	int cpuinfo[4];
	__cpuid(cpuinfo, 1);
	bool HT = (cpuinfo[3] & (1 << 28)) > 0;
	if (HT) numthreads /= 2;
#endif

	clear();

	float dz = (set.cdom.zmax - set.cdom.zmin) / set.cdom.zrez;
	float dt = (set.cdom.tmax - set.cdom.tmin) / set.cdom.trez;
	zmin = set.cdom.zmin;
	zmax = set.cdom.zmax;
	float _zmin = zmin;
	unsigned int tzDelta = set.cdom.zrez / numthreads;
	unsigned int tzLast = set.cdom.zrez - (numthreads - 1) * tzDelta + 1;

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
			d->set = set;
			d->dz = dz;
			d->dt = dt;
			d->zmin = _zmin;
			d->ID = ID;

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
			verticies.insert(verticies.end(), data[i]->func.begin(), data[i]->func.end());

			if(set.color == color_by::gradient) {
				colors.insert(colors.end(), data[i]->grad.begin(), data[i]->grad.end());
				data[i]->grad.clear();
			}			
		}
	}

	if (success) {
		xmin = FLT_MAX; xmax = -FLT_MAX;
		ymin = FLT_MAX; ymax = -FLT_MAX;
		for (unsigned int i = 0; i < threads.size(); i++) {
			xmin = std::min(data[i]->gxmin, xmin);
			xmax = std::max(data[i]->gxmax, xmax);
			ymin = std::min(data[i]->gymin, ymin);
			ymax = std::max(data[i]->gymax, ymax);
		}

		normalize();
		clampInfBounds();
		generateIndiciesAndNormals();
	}

	for (gendata* g : data)
		delete g;
}

spr_graph::spr_graph(int id) : graph(id) {
	type = graph_spherical;
	set.sdom = { 0, 2 * (float)CONST_PI, 0, (float)CONST_PI, 200, 200 };
}

void spr_graph::genthread(gendata* d) {

	double p = d->pmin;
	double t = d->set.sdom.tmin;

	exprtk::symbol_table<double> table = default_table();
	std::string phi_utf("φ");
	std::string theta_utf("θ");

	table.add_variable(utf8_to_wstring(phi_utf),p);
	table.add_variable(utf8_to_wstring(theta_utf),t);

	exprtk::expression<double> expr = make_expression(d->eq, table, d->s->ui);
	if(d->s->ui.error_shown) {
		d->success = false;
		return;
	}

	for (int tp = 0; tp < d->tprez; tp++, p += d->dp) {
		t = d->set.sdom.tmin;
		for (int tt = 0; tt <= d->set.sdom.trez; tt++, t += d->dt) {
			double r = select_calc(d->set.calc, expr, p, t);

			if(d->set.color == color_by::gradient) {
				glm::vec2 xy = get_grad(expr, p, t);
				d->grad.push_back(glm::abs(glm::vec3(xy, 0)));
			}

			double x = r * cos(t) * sin(p);
			double y = r * sin(t) * sin(p);
			double z = r * cos(p);

			d->gzmin = std::min(d->gzmin, (float)z);
			d->gzmax = std::max(d->gzmax, (float)z);
			d->gymin = std::min(d->gymin, (float)y);
			d->gymax = std::max(d->gymax, (float)y);
			d->gxmin = std::min(d->gxmin, (float)x);
			d->gxmax = std::max(d->gxmax, (float)x);

			d->func.push_back(glm::vec3(x, y, z));
		}
	}
	d->success = true;
}

void spr_graph::generate(state* s) {
	unsigned int numthreads = std::thread::hardware_concurrency();
#ifdef _MSC_VER
	int cpuinfo[4];
	__cpuid(cpuinfo, 1);
	bool HT = (cpuinfo[3] & (1 << 28)) > 0;
	if (HT) numthreads /= 2;
#endif

	clear();

	float dt = (set.sdom.tmax - set.sdom.tmin) / set.sdom.trez;
	float dp = (set.sdom.pmax - set.sdom.pmin) / set.sdom.prez;
	float pmin = set.sdom.pmin;
	unsigned int tpDelta = set.sdom.prez / numthreads;
	unsigned int tpLast = set.sdom.prez - (numthreads - 1) * tpDelta + 1;

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
			d->set = set;
			d->eq = eq_str;
			d->dt = dt;
			d->dp = dp;
			d->pmin = pmin;
			d->ID = ID;

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
			verticies.insert(verticies.end(), data[i]->func.begin(), data[i]->func.end());

			if(set.color == color_by::gradient) {
				colors.insert(colors.end(), data[i]->grad.begin(), data[i]->grad.end());
				data[i]->grad.clear();
			}				
		}
	}

	if (success) {
		xmin = FLT_MAX; xmax = -FLT_MAX;
		ymin = FLT_MAX; ymax = -FLT_MAX;
		zmin = FLT_MAX; zmax = -FLT_MAX;
		for (unsigned int i = 0; i < threads.size(); i++) {
			xmin = std::min(data[i]->gxmin, xmin);
			xmax = std::max(data[i]->gxmax, xmax);
			ymin = std::min(data[i]->gymin, ymin);
			ymax = std::max(data[i]->gymax, ymax);
			zmin = std::min(data[i]->gzmin, zmin);
			zmax = std::max(data[i]->gzmax, zmax);
		}

		normalize();
		clampInfBounds();
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

	clear();

	double t = set.pdom.tmin;

	exprtk::symbol_table<double> table;
	table.add_variable(L"t",t);

	exprtk::expression<double> expr_x = make_expression(eqx, table, s->ui);
	exprtk::expression<double> expr_y = make_expression(eqy, table, s->ui);
	exprtk::expression<double> expr_z = make_expression(eqz, table, s->ui);
	if(s->ui.error_shown) {
		return;
	}

	for (int tstep = 0; tstep < set.pdom.trez; tstep++, t += (set.pdom.tmax - set.pdom.tmin) / set.pdom.trez) {
		double x = select_calc(set.calc, expr_x, t);
		double y = select_calc(set.calc, expr_y, t);
		double z = select_calc(set.calc, expr_z, t);

		xmin = std::min(xmin, (float)x);
		xmax = std::max(xmax, (float)x);
		ymin = std::min(ymin, (float)y);
		ymax = std::max(ymax, (float)y);
		zmin = std::min(zmin, (float)z);
		zmax = std::max(zmax, (float)z);

		verticies.push_back(glm::vec3(x, y, z));
	}
	clampInfBounds();		
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
