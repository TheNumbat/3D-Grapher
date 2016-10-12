#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include <vector>
#include <queue>

using namespace std;

typedef int op;

enum operators : op {
	add = '+',
	subtract = '-',
	multiply = '*',
	divide = '/',
	power = '^',
	var_x = 'x',
	var_y = 'y',
	open_p = '(',
	close_p = ')',
	num_break = 1000,
	op_sqrt = 1001,
	op_sin = 1002,
	op_cos = 1003,
	op_tan = 1004
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
		case var_x:
			s.push(x);
			break;
		case var_y:
			s.push(y);
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
	case divide: return 1;
	case power: return 2;
	case var_x:
	case var_y: return 100;
	}
}

void in(istream& in, vector<op>& EQ) {
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
					if (buf != open_p)
						EQ.push_back(buf);
					s.pop();
				} while (s.size() && buf != open_p);
			}
			while (s.size()) {
				if (s.top() == open_p) {
					s.pop();
					continue;
				}
				EQ.push_back(s.top());
				s.pop();
				break;
			}
			break;
		case multiply:
		case divide:
		case add:
		case subtract:
		case power:
			ins = true;
			queued = false; added = false;
			while (s.size() && precedence(s.top()) == precedence(buf)) {
				q.push(s.top());
				s.pop();
				queued = true;
			}
			if (queued)
				EQ.push_back(buf);
			while (q.size()) {
				s.push(q.front());
				q.pop();
			}
			while (s.size() && precedence(s.top()) > precedence(buf)) {
				EQ.push_back(s.top());
				s.pop();
				added = true;
			}
			if (!queued)
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
		default:
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
				s.push('(');
			}
			else {
				while (num(buf)) {
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
		EQ.push_back(s.top());
		s.pop();
	}
}

int main(int argc, char** args) {
	vector<op> eq;
	stringstream ss;
	string s;
	float x, y;

	while (true) {
		getline(cin, s);
		ss << s;
		ss >> x >> y;
		in(ss, eq);
		ss.clear();

		for (op c : eq) {
			if (c == op_sqrt)
				cout << "Sq";
			else if (c == op_sin)
				cout << "Sn";
			else if (c == op_cos)
				cout << "Cs";
			else if (c == op_tan)
				cout << "Tn";
			else
				cout << (char)c;
		}
		cout << endl;

		float result = eval(eq, x, y);
		cout << x << " " << y << " " << result << endl;

		eq.clear();
	}

	system("pause");
	return 0;
}