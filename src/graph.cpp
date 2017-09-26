

GLfloat axes[] = {
   -10.0f,  0.0f ,  0.0f ,		1.0f, 0.0f, 0.0f,
	10.0f,  0.0f ,  0.0f ,		1.0f, 0.0f, 0.0f,

	0.0f , -10.0f,  0.0f ,		0.0f, 1.0f, 0.0f,
	0.0f ,  10.0f,  0.0f ,		0.0f, 1.0f, 0.0f,

	0.0f ,  0.0f , -10.0f,		0.0f, 0.0f, 1.0f,
	0.0f ,  0.0f ,  10.0f,		0.0f, 0.0f, 1.0f
};

void regengraph(state* s, int index) {

	if (!s->graphs[index]->update_eq(s)) return;
	printeq(cout, s->graphs[index]->eq);

	Uint64 start = SDL_GetPerformanceCounter();
	s->graphs[index]->generate(s);
	Uint64 end = SDL_GetPerformanceCounter();
	cout << "time: " << (float)(end - start) / SDL_GetPerformanceFrequency() << endl;

	updateAxes(s);
	s->graphs[index]->send();
}

int getIndex(state* s, int ID) {
	auto entry = find_if(s->graphs.begin(), s->graphs.end(), [ID](graph* g) -> bool {return g->ID == ID; });
	if (entry == s->graphs.end()) {
		return -1;
	}
	else {
		int pos = (int)(entry - s->graphs.begin());
		return pos;
	}
}

void regenall(state* s) {
	bool gen = false;
	for (int i = 0; i < (int)s->graphs.size(); i++) {
		regengraph(s, i);
		if (s->graphs[i]->verticies.size()) {
			gen = true;
		}
	}
	if (gen) {
		updateAxes(s);
		//resetCam(s);
	}
}

void updateAxes(state* s) {
	float xmin = FLT_MAX, xmax = -FLT_MAX;
	float ymin = FLT_MAX, ymax = -FLT_MAX;
	float zmin = FLT_MAX, zmax = -FLT_MAX;

	bool found_a_graph = false;
	for (graph* g : s->graphs) {
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

	glBindVertexArray(s->axisVAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, s->axisVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);
	}
}

void resetCam(state* s) {
	s->c_3d_static.radius = std::max(axes[y_max] - axes[y_min], axes[x_max] - axes[x_min]);
	s->c_3d_static.lookingAt.x = (axes[x_max] + axes[x_min]) / 2;
	s->c_3d_static.lookingAt.z = (axes[y_max] + axes[y_min]) / -2;
	s->c_3d_static.updatePos();

	s->c_3d.pos.x = (axes[x_max] + axes[x_min]) / 2;
	s->c_3d.pos.z = (axes[y_max] + axes[y_min]) / -2;
	s->c_3d.updateFront();
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * normals.size(), normals.size() ? &normals[0] : NULL, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indicies.size(), indicies.size() ? &indicies[0] : NULL, GL_STATIC_DRAW);

	glBindVertexArray(0);
}

bool graph::update_eq(state* s) {
	vector<op> new_eq;

	try { in(utf8_to_wstring(eq_str), new_eq); }
	catch (runtime_error e) {
		s->error_shown = true;
		s->error = e.what();
		return false;
	}

	eq = new_eq;
	return true;
}

void graph::draw(state* s, mat4 model, mat4 view, mat4 proj) {
	if(!verticies.size()) return;

	glBindVertexArray(VAO);
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);

		mat4 modelviewproj = proj * view * model;

		if (set.lighting) {
			s->graph_s_light.use();

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

			if (s->c_set.camtype == cam_3d) {

				glUniform3f(s->graph_s_light.getUniform("lightPos"), s->c_3d.pos.x, s->c_3d.pos.y, s->c_3d.pos.z);
			}
			else {
				glUniform3f(s->graph_s_light.getUniform("lightPos"), s->c_3d_static.pos.x, s->c_3d_static.pos.y, s->c_3d_static.pos.z);
			}
		}
		else {
			s->graph_s.use();

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);

			glUniformMatrix4fv(s->graph_s.getUniform("modelviewproj"), 1, GL_FALSE, value_ptr(modelviewproj));

			glUniform4f(s->graph_s.getUniform("vcolor"), 0.8f, 0.8f, 0.8f, set.opacity);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glPolygonOffset(1.0f, 0.0f);
		glDrawElements(GL_TRIANGLES, (int)indicies.size(), GL_UNSIGNED_INT, (void*)0);

		if (set.wireframe) {
			glDisable(GL_BLEND);

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glPolygonOffset(0.0f, 0.0f);

			if (set.lighting)
				glUniform4f(s->graph_s_light.getUniform("vcolor"), 0.0f, 0.0f, 0.0f, set.opacity);
			else
				glUniform4f(s->graph_s.getUniform("vcolor"), 0.0f, 0.0f, 0.0f, set.opacity);

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

	vec3 norm;
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
				vec3 one(x2 - x1, y2 - y1, z2 - z1);
				vec3 two(x3 - x1, y3 - y1, z3 - z1);
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

	float x = g->xmin;
	for (int tx = 0; tx < g->txrez; tx++, x += g->dx) {
		float y = g->dom.ymin;
		for (int ty = 0; ty <= g->dom.yrez; ty++, y += g->dy) {
			float z;
			try { z = eval(g->eq, { { 'x',x },{ 'y',y } }); }
			catch (runtime_error e) {
				g->s->error_shown = true;
				g->s->error = e.what();
				g->success = false;
				return;
			}

			if (z < g->zmin) g->zmin = z;
			if (z > g->zmax) g->zmax = z;

			g->ret.push_back(x);
			g->ret.push_back(y);
			g->ret.push_back(z);
		}
	}
	g->success = true;
}

void fxy_graph::generate(state* s) {
	unsigned int numthreads = thread::hardware_concurrency();
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

	vector<thread> threads;
	vector<gendata*> data;
	for (unsigned int i = 0; i < numthreads; i++) {
		if (txDelta || i == numthreads - 1) {
			gendata* d = new gendata;

			if (i == numthreads - 1)
				d->txrez = txLast;
			else
				d->txrez = txDelta;

			d->s = s;
			d->dom = set.rdom;
			d->eq = eq;
			d->dx = dx;
			d->dy = dy;
			d->xmin = _xmin;
			d->ID = ID;

			data.push_back(d);
			threads.push_back(thread(genthread, data.back()));

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
	set.cdom = { 0, 1, 0, 2 * val_pi, 200, 200 };
}

void cyl_graph::genthread(gendata* g) {
	float z = g->zmin;
	for (int tz = 0; tz < g->tzrez; tz++, z += g->dz) {
		float t = g->dom.tmin;
		for (int tt = 0; tt <= g->dom.trez; tt++, t += g->dt) {
			float r;
			try { r = eval(g->eq, { { 'z',z },{ 952,t } }); }
			catch (runtime_error e) {
				g->s->error_shown = true;
				g->s->error = e.what();			
				g->success = false;
				return;
			}

			float x = r * cos(t);
			float y = r * sin(t);
			if (x < g->gxmin) g->gxmin = x;
			if (x > g->gxmax) g->gxmax = x;
			if (y < g->gymin) g->gymin = y;
			if (y > g->gymax) g->gymax = y;

			g->ret.push_back(x);
			g->ret.push_back(y);
			g->ret.push_back(z);
		}
	}
	g->success = true;
}

void cyl_graph::generate(state* s) {
	unsigned int numthreads = thread::hardware_concurrency();
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

	vector<thread> threads;
	vector<gendata*> data;
	for (unsigned int i = 0; i < numthreads; i++) {
		if (tzDelta || i == numthreads - 1) {
			gendata* d = new gendata;

			if (i == numthreads - 1)
				d->tzrez = tzLast;
			else
				d->tzrez = tzDelta;

			d->s = s;
			d->eq = eq;
			d->dom = set.cdom;
			d->dz = dz;
			d->dt = dt;
			d->zmin = _zmin;
			d->ID = ID;

			data.push_back(d);
			threads.push_back(thread(genthread, data.back()));

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
	set.sdom = { 0, 2 * val_pi, 0, val_pi, 200, 200 };
}

void spr_graph::genthread(gendata* g) {

	float p = g->pmin;
	for (int tp = 0; tp < g->tprez; tp++, p += g->dp) {
		float t = g->dom.tmin;
		for (int tt = 0; tt <= g->dom.trez; tt++, t += g->dt) {
			float r;
			try { r = eval(g->eq, { { 952,t },{ 966,p } }); }
			catch (runtime_error e) {
				g->s->error_shown = true;
				g->s->error = e.what();				
				g->success = false;
				return;
			}

			float x = r * cos(t) * sin(p);
			float y = r * sin(t) * sin(p);
			float z = r * cos(p);

			if (z < g->gzmin) g->gzmin = z;
			if (z > g->gzmax) g->gzmax = z;
			if (y < g->gymin) g->gymin = y;
			if (y > g->gymax) g->gymax = y;
			if (x < g->gxmin) g->gxmin = x;
			if (x > g->gxmax) g->gxmax = x;

			g->ret.push_back(x);
			g->ret.push_back(y);
			g->ret.push_back(z);
		}
	}
	g->success = true;
}

void spr_graph::generate(state* s) {
	unsigned int numthreads = thread::hardware_concurrency();
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

	vector<thread> threads;
	vector<gendata*> data;
	for (unsigned int i = 0; i < numthreads; i++) {
		if (tpDelta || i == numthreads - 1) {
			gendata* d = new gendata;

			if (i == numthreads - 1)
				d->tprez = tpLast;
			else
				d->tprez = tpDelta;

			d->s = s;
			d->dom = set.sdom;
			d->eq = eq;
			d->dt = dt;
			d->dp = dp;
			d->pmin = pmin;
			d->ID = ID;

			data.push_back(d);
			threads.push_back(thread(genthread, data.back()));

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
	sx.resize(1000, 0);
	sy.resize(1000, 0);
	sz.resize(1000, 0);
	set.pdom = { 0, 10, 100 };
}

bool para_curve::update_eq(state* s) {
	vector<op> new_eqx, new_eqy, new_eqz;

	try {
		in(utf8_to_wstring(sx), new_eqx);
		in(utf8_to_wstring(sy), new_eqy);
		in(utf8_to_wstring(sz), new_eqz);
	}
	catch (runtime_error e) {
		s->error_shown = true;
		s->error = e.what();		
		return false;
	}

	eqx = new_eqx;
	eqy = new_eqy;
	eqz = new_eqz;
	return true;
}

void para_curve::generate(state* s) {

	xmin = ymin = zmin = FLT_MAX;
	xmax = ymax = zmax = -FLT_MAX;

	verticies.clear();
	float t = set.pdom.tmin;
	for (int tstep = 0; tstep < set.pdom.trez; tstep++, t += (set.pdom.tmax - set.pdom.tmin) / set.pdom.trez) {
		float x, y, z;
		try { 
			x = eval(eqx, { { 't',t } });
			y = eval(eqy, { { 't',t } });
			z = eval(eqz, { { 't',t } });
		}
		catch (runtime_error e) {
			s->error_shown = true;
			s->error = e.what();			
			return;
		}
		if (z < zmin) zmin = z;
		if (z > zmax) zmax = z;
		if (y < ymin) ymin = y;
		if (y > ymax) ymax = y;
		if (x < xmin) xmin = x;
		if (x > xmax) xmax = x;

		verticies.push_back(x);
		verticies.push_back(y);
		verticies.push_back(z);
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

void para_curve::draw(state* s, mat4 model, mat4 view, mat4 proj) {
	glBindVertexArray(VAO);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	mat4 modelviewproj = proj * view * model;

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
