
#pragma once

#include <iostream>
#include <vector>

using namespace std;

const double val_e = 2.7182818284590452353602874713527;
const double val_pi = 3.141592653589793238462643383279;

typedef int op;

void welcome(ostream& out);
float eval(const vector<op>& EQ, float x, float y);
bool in(istream& in, vector<op>& EQ);
void printeq(ostream& out, vector<op> eq);

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
	op_exptwo = 1010,
	op_ceil = 1011,
	op_floor = 1012,
	op_ln = 1013,
	op_log = 1014,
	op_log2 = 1015,

	// variables
	var_x = 'x',
	var_y = 'y',

	// constants
	const_pi = 'P',
	const_e = 'E'
};

/////////////////////////////////////////////////////////////////////////////

#include <string>
#include <stack>
#include <queue>

void welcome(ostream& out) {
	out << endl
		<< "   Expression Eval" << endl
		<< "   ===============" << endl
		<< endl
		<< "   Variables: x, y" << endl
		<< "   Operators: +, -, %, *, /, ^, (, )" << endl
		<< "   Constants: E = 2.71... P = 3.14..." << endl
		<< "   Functions: sqrt, sin, cos, tan, asin, acos, atan, abs, exp, exptwo, ceil, floor, ln, log, log2" << endl
		<< endl
		<< "   Format: x y <exp>" << endl
		<< "   Example: 3 4 x / y * (e ^ x - y)" << endl
		<< "   Will try to evaluate unbalanced parenthesis" << endl
		<< "   Type \"exit\" to quit" << endl
		<< endl;
}

#define get2() two = s.top(); \
			   s.pop(); \
			   one = s.top(); \
			   s.pop();

#define get1() one = s.top(); \
			   s.pop();

float eval(const vector<op>& EQ, float x, float y) {
	stack<float> s;
	float one = 0, two = 0, result = 0;
	int size = EQ.size();
	for (int index = 0; index < size; index++) {
		switch (EQ[index]) {
		case add:
			get2();
			result = one + two;
			s.push(result);
			break;
		case subtract:
			get2();
			result = one - two;
			s.push(result);
			break;
		case multiply:
			get2();
			result = one * two;
			s.push(result);
			break;
		case divide:
			get2();
			result = one / two;
			s.push(result);
			break;
		case modulo:
			get2();
			result = fmod(one, two);
			s.push(result);
			break;
		case power:
			get2();
			result = pow(one, two);
			s.push(result);
			break;
		case op_sqrt:
			get1();
			result = sqrt(one);
			s.push(result);
			break;
		case op_sin:
			get1();
			result = sin(one);
			s.push(result);
			break;
		case op_cos:
			get1();
			result = cos(one);
			s.push(result);
			break;
		case op_tan:
			get1();
			result = tan(one);
			s.push(result);
			break;
		case op_asin:
			get1();
			result = asin(one);
			s.push(result);
			break;
		case op_acos:
			get1();
			result = acos(one);
			s.push(result);
			break;
		case op_atan:
			get1();
			result = atan(one);
			s.push(result);
			break;
		case op_abs:
			get1();
			result = abs(one);
			s.push(result);
			break;
		case op_exp:
			get1();
			result = exp(one);
			s.push(result);
			break;
		case op_exptwo:
			get1();
			result = exp2(one);
			s.push(result);
			break;
		case op_ceil:
			get1();
			result = ceil(one);
			s.push(result);
			break;
		case op_floor:
			get1();
			result = floor(one);
			s.push(result);
			break;
		case op_ln:
			get1();
			result = log(one);
			s.push(result);
			break;
		case op_log:
			get1();
			result = log10(one);
			s.push(result);
			break;
		case op_log2:
			get1();
			result = log2(one);
			s.push(result);
			break;
		case var_x:
			s.push(x);
			break;
		case var_y:
			s.push(y);
			break;
		case const_e:
			s.push(val_e);
			break;
		case const_pi:
			s.push(val_pi);
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
		if (ins)
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
				if (s.size()) {
					EQ.push_back(s.top());
					s.pop();
				}
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
				else if (str == "exptwo")
					s.push(op_exptwo);
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

void printeq(ostream& out, vector<op> eq) {
	for (op c : eq) {
		if (c == op_sqrt)
			out << "sqrt";
		else if (c == op_sin)
			out << "sin";
		else if (c == op_cos)
			out << "cos";
		else if (c == op_tan)
			out << "tan";
		else if (c == op_asin)
			out << "asin";
		else if (c == op_acos)
			out << "acos";
		else if (c == op_atan)
			out << "atan";
		else if (c == op_abs)
			out << "abs";
		else if (c == op_exp)
			out << "exp";
		else if (c == op_exptwo)
			out << "exptwo";
		else if (c == op_ceil)
			out << "ceil";
		else if (c == op_floor)
			out << "floor";
		else if (c == op_ln)
			out << "ln";
		else if (c == op_log)
			out << "log";
		else if (c == op_log2)
			out << "log2";
		else
			out << (char)c;
	}
	out << endl;
}