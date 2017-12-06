
#pragma once

struct state;

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

double select_calc(calculus calc, const exprtk::expression<double>& expr, double& t);
double select_calc(calculus calc, const exprtk::expression<double>& expr, double& x, double& y);
glm::vec2 get_grad(const exprtk::expression<double>& expr, double& x, double& y);

struct graph {
	graph(int id);
	virtual ~graph();

	virtual void normalize();
	virtual void generateIndiciesAndNormals();

	virtual void generate(state* s) = 0;
	virtual void draw(state* s, glm::mat4 model, glm::mat4 view, glm::mat4 proj);

	void gen();
	void send();
	void clampInfBounds();
	void clear();

	std::vector<GLfloat> verticies;
	std::vector<GLuint> indicies;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> colors;

	int ID;
	std::string eq_str;
	graph_type type;
	
	float xmin, xmax, ymin, ymax, zmin, zmax; // set after generation
	GLuint VAO, VBO, EBO, normVBO, colorVBO;

	graph_settings set;
};

struct para_curve : public graph {
	para_curve(int id);
	void generate(state* s);
	void draw(state* s, glm::mat4 model, glm::mat4 view, glm::mat4 proj);
	void generateIndiciesAndNormals();
	std::string eqx, eqy, eqz;
};

struct fxy_graph : public graph {
	struct gendata {
		gendata() {
			zmin = FLT_MAX;
			zmax = -FLT_MAX;
		};
		std::vector<float> 		func;
		std::vector<glm::vec3> 	grad;
		
		float zmin, zmax, xmin, dx, dy;
		int txrez, ID;
		bool success;
		
		state* s;

		graph_settings set;	
		std::string eq;
	};

	fxy_graph(int id);
	void generate(state* s);
	static void genthread(gendata* g);
};

struct cyl_graph : public graph {
	struct gendata {
		gendata() {
			gxmin = gymin = FLT_MAX;
			gxmax = gymax = -FLT_MAX;
		};
		std::vector<float> func;
		std::vector<glm::vec3> grad;

		float gxmin, gxmax, gymin, gymax, zmin, dz, dt;
		int tzrez, ID;
		bool success;

		state* s;
		graph_settings set;
		std::string eq;
	};

	cyl_graph(int id);
	void generate(state* s);
	static void genthread(gendata* g);
};

struct spr_graph : public graph {
	struct gendata {
		gendata() {
			gxmin = gymin = gzmin = FLT_MAX;
			gxmax = gymax = gzmax = -FLT_MAX;
		};
		std::vector<float> func;
		std::vector<glm::vec3> grad;
		
		float gxmin, gxmax, gymin, gymax, gzmin, gzmax, pmin, dt, dp;
		int tprez, ID;
		bool success;

		state* s;
		std::string eq;
		graph_settings set;
	};

	spr_graph(int id);
	void generate(state* s);
	static void genthread(gendata* g);
};
