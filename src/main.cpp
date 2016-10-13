#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include <vector>
#include <queue>

using namespace std;

const double e = 2.7182818284590452353602874713527;
const double pi = 3.141592653589793238462643383279;

typedef int op;

enum operators : op {
	// operators
	add = '+',
	subtract = '-',
	modulo = '%',
	multiply = '*',
	divide = '/',
	power = '^',
	open_p = '(',
	close_p = ')',
	num_break = 1000,
	
	// common functions
	op_sqrt = 1001,
	op_sin = 1002,
	op_cos = 1003,
	op_tan = 1004,
	op_asin = 1005,
	op_acos = 1006,
	op_atan = 1007,
	op_abs = 1008,
	op_exp = 1009,
	op_exp2 = 1010,
	op_ceil = 1011,
	op_floor = 1012,
	op_ln = 1013,
	op_log = 1014,
	op_log2 = 1015,

	// variables
	var_x = 'x',
	var_y = 'y',

	// constants
	const_pi = 'p',
	const_e = 'e'
};

#define get() two = s.top(); \
			  s.pop(); \
			  one = s.top(); \
			  s.pop();

float eval(vector<op> EQ, float x, float y) {
	stack<float> s;
	float one = 0, two = 0, result = 0;
	for (int index = 0; index < EQ.size(); index++) {
		switch (EQ[index]) {
		case add:
			get();
			result = one + two;
			s.push(result);
			break;
		case subtract:
			get();
			result = one - two;
			s.push(result);
			break;
		case multiply:
			get();
			result = one * two;
			s.push(result);
			break;
		case divide:
			get();
			result = one / two;
			s.push(result);
			break;
		case power:
			get();
			result = pow(one, two);
			s.push(result);
			break;
		case op_sqrt:
			one = s.top();
			s.pop();
			result = sqrt(one);
			s.push(result);
			break;
		case op_sin:
			one = s.top();
			s.pop();
			result = sin(one);
			s.push(result);
			break;
		case op_cos:
			one = s.top();
			s.pop();
			result = cos(one);
			s.push(result);
			break;
		case op_tan:
			one = s.top();
			s.pop();
			result = tan(one);
			s.push(result);
			break;
		case op_asin:
			one = s.top();
			s.pop();
			result = asin(one);
			s.push(result);
			break;
		case op_acos:
			one = s.top();
			s.pop();
			result = acos(one);
			s.push(result);
			break;
		case op_atan:
			one = s.top();
			s.pop();
			result = atan(one);
			s.push(result);
			break;
		case op_abs:
			one = s.top();
			s.pop();
			result = abs(one);
			s.push(result);
			break;
		case op_exp:
			one = s.top();
			s.pop();
			result = exp(one);
			s.push(result);
			break;
		case op_exp2:
			one = s.top();
			s.pop();
			result = exp2(one);
			s.push(result);
			break;
		case op_ceil:
			one = s.top();
			s.pop();
			result = ceil(one);
			s.push(result);
			break;
		case op_floor:
			one = s.top();
			s.pop();
			result = floor(one);
			s.push(result);
			break;
		case op_ln:
			one = s.top();
			s.pop();
			result = log(one);
			s.push(result);
			break;
		case op_log:
			one = s.top();
			s.pop();
			result = log10(one);
			s.push(result);
			break;
		case op_log2:
			one = s.top();
			s.pop();
			result = log2(one);
			s.push(result);
			break;
		case modulo:
			get();
			result = fmod(one, two);
			s.push(result);
			break;
		case var_x:
			s.push(x);
			break;
		case var_y:
			s.push(y);
			break;
		case const_e:
			s.push(e);
			break;
		case const_pi:
			s.push(pi);
			break;
		default:
			int i2 = index;
			string num;
			while (EQ[i2] != num_break) {
				num.push_back(EQ[i2]);
				i2++;
				index++;
			}
			s.push(atof(num.c_str()));
			break;
		}
	}
	return s.top();
}

bool num(char c) {
	return c >= '0' && c <= '9' || c == '.';
}

int precedence(char c) {
	switch (c) {
	case open_p: return -1;
	case add:
	case subtract: return 0;
	case multiply:
	case modulo:
	case divide: return 1;
	case power: return 2;
	default: return 100;
	}
}

bool in(istream& in, vector<op>& EQ) {
	char buf = 0;
	int  pos;
	stack<op> s;
	queue<op> q;
	bool queued = false, added = false, ins = true;
	while (!in.eof()) {
		if(ins)
			in >> buf;
		if (in.eof()) break;
		switch (buf) {
		case open_p:
			ins = true;
			s.push(buf);
			break;
		case close_p:
			ins = true;
			if (s.size()) {
				do {
					buf = s.top();
					if (buf != open_p) {
						EQ.push_back(buf);
					}
					s.pop();
				} while (s.size() && buf != open_p);
			}
			if (s.size()) {
				if (s.top() == open_p) {
					s.pop();
				}
				EQ.push_back(s.top());
				s.pop();
			}
			break;
		case multiply:
		case divide:
		case add:
		case subtract:
		case modulo:
		case power:
			ins = true;
			queued = false; added = false;
			while (s.size() && precedence(s.top()) == precedence(buf)) {
				q.push(s.top());
				s.pop();
				queued = true;
			}
			if (queued) {
				EQ.push_back(q.front());
				q.pop();
			}
			while (q.size()) {
				s.push(q.front());
				q.pop();
			}
			while (s.size() && precedence(s.top()) > precedence(buf)) {
				EQ.push_back(s.top());
				s.pop();
				added = true;
			}
			s.push(buf);
			break;
		case var_x:
			ins = true;
			EQ.push_back('x');
			break;
		case var_y:
			ins = true;
			EQ.push_back('y');
			break;
		case const_e:
			ins = true;
			EQ.push_back(const_e);
			break;
		case const_pi:
			ins = true;
			EQ.push_back(const_pi);
			break;
		default: // functions
			if (!num(buf)) {
				string str;
				getline(in, str, '(');
				str.insert(0, 1, buf);
				if (str == "sqrt")
					s.push(op_sqrt);
				else if (str == "sin")
					s.push(op_sin);
				else if (str == "cos")
					s.push(op_cos);
				else if (str == "tan")
					s.push(op_tan);
				else if (str == "asin")
					s.push(op_asin);
				else if (str == "acos")
					s.push(op_acos);
				else if (str == "atan")
					s.push(op_atan);
				else if (str == "abs")
					s.push(op_abs);
				else if (str == "exp")
					s.push(op_exp);
				else if (str == "exp2")
					s.push(op_exp2);
				else if (str == "ceil")
					s.push(op_ceil);
				else if (str == "floor")
					s.push(op_floor);
				else if (str == "ln")
					s.push(op_ln);
				else if (str == "log")
					s.push(op_log);
				else if (str == "log2")
					s.push(op_log2);
				else {
					if (!in.good())
						cout << "   err: unkown name '" << str << "'" << endl;
					else
						cout << "   err: unkown function '" << str << "()'" << endl;
					return false;
				}
				s.push('(');
			}
			else {
				while (!in.eof() && num(buf)) {
					EQ.push_back(buf);
					in >> buf;
					ins = false;
				}
				EQ.push_back(num_break);
			}
			break;
		}
	}
	while (s.size()) {
		if (s.top() == '(' || s.top() == ')') {
			cout << "   err: unbalanced parenthesis" << endl;
			return false;
		}
		EQ.push_back(s.top());
		s.pop();
	}
	return true;
}

void welcome() {
	cout << endl
		 << "   Expression Eval" << endl
		 << "   ===============" << endl
		 << endl
		 << "   Variables: x, y" << endl
		 << "   Operators: +, -, %, *, /, ^, (, )" << endl
		 << "   Constants: e = 2.71... p = 3.14..." << endl
		 << "   Functions: sqrt, sin, cos, tan, asin, acos, atan, abs, exp, exp2, ceil, floor, ln, log, log2" << endl
		 << endl
		 << "   Format: x y <exp>" << endl
		 << "   Example: 3 4 x / y * (e ^ x - y)" << endl
		 << "   Will try to evaluate unbalanced parenthesis" << endl
		 << "   Type \"exit\" to quit" << endl
		 << endl;
}

void printeq(vector<op> eq) {
	for (op c : eq) {
		if (c == op_sqrt)
			cout << "sqrt";
		else if (c == op_sin)
			cout << "sin";
		else if (c == op_cos)
			cout << "cos";
		else if (c == op_tan)
			cout << "tan";
		else if (c == op_asin)
			cout << "asin";
		else if (c == op_acos)
			cout << "acos";
		else if (c == op_atan)
			cout << "atan";
		else if (c == op_abs)
			cout << "abs";
		else if (c == op_exp)
			cout << "exp";
		else if (c == op_exp2)
			cout << "exp2";
		else if (c == op_ceil)
			cout << "ceil";
		else if (c == op_floor)
			cout << "floor";
		else if (c == op_ln)
			cout << "ln";
		else if (c == op_log)
			cout << "log";
		else if (c == op_log2)
			cout << "log2";
		else
			cout << (char)c;
	}
	cout << endl;
}

int main(int argc, char** args) {
	welcome();
	
	vector<op> eq;
	string s;
	float x, y;

	while (true) {
		stringstream ss;
		cout << "   : ";
		getline(cin, s);
		if (s == "exit") break;

		ss << s;
		ss >> x >> y;
		if (in(ss, eq)) {
			//printeq(eq);

			float result = eval(eq, x, y);
			cout << "    > " << result << endl;
		}

		eq.clear();
	}

	return 0;
}