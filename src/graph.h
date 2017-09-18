
#pragma once

struct state;

struct t_range {
	float tmin, tmax;
	int trez;
};

enum graph_type {
	graph_func,				// done
	graph_cylindrical,		// done
	graph_spherical,		// done
	graph_surface,
	graph_para_curve,		// done except domain
	graph_para_surface,
	graph_vec_feild
};

const int x_min = 0;
const int x_max = 6;
const int y_min = 13;
const int y_max = 19;
const int z_min = 26;
const int z_max = 32;

extern GLfloat axes[];

struct graph {
	graph(int id);
	virtual ~graph();

	virtual void normalize(state* s);
	virtual void generate(state* s) = 0;
	virtual void generateIndiciesAndNormals(state* s);
	virtual void draw(state* s, mat4 model, mat4 view, mat4 proj);
	virtual bool update_eq(state* s);

	void gen();
	void send();

	vector<GLfloat> verticies;
	vector<GLuint> indicies;
	vector<vec3> normals;

	int ID;
	string eq_str;
	vector<op> eq;
	graph_type type;
	float zmin, zmax, rel_opactiy;
	GLuint VAO, VBO, EBO, normVBO;
};

struct para_curve : public graph {
	para_curve(int id, string sx = " ", string sy = " ", string sz = " ");
	void generate(state* s);
	void draw(state* s, mat4 model, mat4 view, mat4 proj);
	void generateIndiciesAndNormals(state* s);
	bool update_eq(state* s);
	string sx, sy, sz;
	vector<op> eqx, eqy, eqz;
	t_range range;
};

struct fxy_graph : public graph {
	struct gendata {
		gendata() {
			zmin = FLT_MAX;
			zmax = -FLT_MAX;
		};
		state* s;
		vector<float> ret;
		float zmin, zmax, xmin, dx, dy;
		int txrez, ID;
		bool success;
	};

	fxy_graph(int id);
	void generate(state* s);
	static void genthread(gendata* g);
};

struct cyl_graph : public graph {
	struct gendata {
		gendata() {
			gzmin = FLT_MAX;
			gzmax = -FLT_MAX;
		};
		state* s;
		vector<float> ret;
		float gzmin, gzmax, zmin, dz, dt;
		int tzrez, ID;
		bool success;
	};

	cyl_graph(int id);
	void generate(state* s);
	static void genthread(gendata* g);
};

struct spr_graph : public graph {
	struct gendata {
		gendata() {
			zmin = FLT_MAX;
			zmax = -FLT_MAX;
		};
		state* s;
		vector<float> ret;
		float zmin, zmax, pmin, dt, dp;
		int tprez, ID;
		bool success;
	};

	spr_graph(int id);
	void generate(state* s);
	static void genthread(gendata* g);
};

void updateAxes(state* s);
void resetCam(state* s);
void regengraph(state* s, int index);
void regenall(state* s);
int getIndex(state* s, int ID);
