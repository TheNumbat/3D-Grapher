
#include "exp.h"
#include <string>
#include <stack>
#include <queue>
#include <algorithm>
#include <exception>
#include <math.h>

using namespace std;

bool isop(char c) {
	return c == open_p || c == add || c == subtract || c == op_neg ||
		   c == multiply || c == modulo || c == divide || c == power;
}

bool isnonfunc(char c) {
	return c == close_p || c == add || c == subtract || c == op_neg ||
		   c == multiply || c == modulo || c == divide || c == power;
}

bool num(char c) {
	return c >= '0' && c <= '9' || c == '.';
}

#define get2() if(s.size() > 1) {two = s.top();s.pop();one = s.top();s.pop();} \
			   else {throw runtime_error("ERROR: malformed expression!");}

#define get1() if(s.size()) {one = s.top();s.pop();} \
			   else {throw runtime_error("ERROR: malformed expression!");}

float eval(const vector<op>& EQ, vector<pair<char, float>> vars) {
	if (!EQ.size()) throw runtime_error("ERROR: empty expression!");
	stack<float> s;
	float one = 0, two = 0, result = 0;
	for (unsigned int index = 0; index < EQ.size(); index++) {
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
			result = fabs(one);
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
		case op_sec:
			get1();
			result = 1.0f / cos(one);
			s.push(result);
			break;
		case op_csc:
			get1();
			result = 1.0f / sin(one);
			s.push(result);
			break;
		case op_cot:
			get1();
			result = 1.0f / tan(one);
			s.push(result);
		case op_asec:
			get1(); 
			result = acos(1.0f / one);
			s.push(result);
			break;
		case op_acsc:
			get1();
			result = asin(1.0f / one);
			s.push(result);
			break;
		case op_acot:
			get1();
			result = atan(1.0f / one);
			s.push(result);
			break;
		case op_neg:
			get1();
			result = -one;
			s.push(result);
			break;
		case var: {
			char v = EQ[index + 1];
			auto entry = find_if(vars.begin(), vars.end(), [v](const pair<char, float>& var) -> bool { return var.first == v; });
			if (entry == vars.end()) {
				throw runtime_error((string)"ERROR: variable " + v + " not recognized!");
			}
			else {
				s.push(entry->second);
			}
			index++;
			break;
		}
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
			s.push((float)atof(num.c_str()));
			break;
		}
	}
	result = s.top();
	s.pop();
	if (s.size()) throw runtime_error("ERROR: malformed expression!");
	return result;
}

int precedence(char c) {
	switch (c) {
	case open_p: return -1;
	case op_neg:
	case add:
	case subtract: return 0;
	case multiply:
	case modulo:
	case divide: return 1;
	case power: return 2;
	default: return 100;
	}
}

void in(string str, vector<op>& EQ) {
	if (str == " " || !str.size()) throw runtime_error("ERROR: blank string!");

	for (int ind = 0; ind < (int)str.size(); ind++) {
		if (str[ind] == '-' && (ind == 0 || isop(str[ind - 1]))) str[ind] = op_neg;
		if (str[ind] == ' ') {
			str.erase(ind, 1);
			ind--;
		}
	}

	char buf = 0;
	stack<op> s;
	queue<op> q;
	stringstream in;
	in << str;
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
				int cur;
				do {
					cur = s.top();
					if (cur != open_p) {
						EQ.push_back(cur);
					}
					s.pop();
				} while (s.size() && cur != open_p);
			}
			break;
		case multiply:
		case divide:
		case add:
		case modulo:
		case power:
		case subtract:
			ins = true;
			while (s.size() && precedence(s.top()) >= precedence(buf)) {
				EQ.push_back(s.top());
				s.pop();
				added = true;
			}
			s.push(buf);
			break;
		case op_neg:
			ins = true;
			s.push(buf);
			break;
		case const_pi:
		case const_e:
			ins = true;
			EQ.push_back(buf);
			break;
		case ',':
			ins = true;
			break;
		default: // Functions, variables, and numbers
			if (!num(buf)) {
				if (in.peek() != EOF && !isnonfunc(in.peek())) {
					string str;
					getline(in, str, '(');
					str.insert(0, 1, buf);
					// Test function name
					{
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
						else if (str == "sec")
							s.push(op_sec);
						else if (str == "csc")
							s.push(op_csc);
						else if (str == "cot")
							s.push(op_cot);
						else if (str == "asec")
							s.push(op_asec);
						else if (str == "acsc")
							s.push(op_acsc);
						else if (str == "acot")
							s.push(op_acot);
						else {
							if (!in.good())
								throw runtime_error("ERROR: unknown name '" + str + "'");
							else
								throw runtime_error("ERROR: unknown function '" + str + "()'");
						}
					}
					s.push('(');
				}
				else {
					ins = true;
					EQ.push_back(var);
					EQ.push_back(buf);
				}
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
			throw runtime_error("ERROR: unbalanced parenthesis!");
		}
		EQ.push_back(s.top());
		s.pop();
	}
	if (!EQ.size()) {
		throw runtime_error("ERROR: empty expression!");
	}
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
		else if (c == op_sec)
			out << "sec";
		else if (c == op_csc)
			out << "csc";
		else if (c == op_cot)
			out << "cot";
		else if (c == op_asec)
			out << "asec";
		else if (c == op_acsc)
			out << "acsc";
		else if (c == op_acot)
			out << "acot";
		else if (c == op_neg)
			out << "neg";
		else if (c == var)
			out << "var";
		else
			out << (char)c;
		out << " ";
	}
	out << endl;
}
