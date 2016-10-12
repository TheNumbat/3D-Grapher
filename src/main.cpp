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
	close_p = ')'
};

#define get() one = s.top(); \
			  s.pop(); \
			  two = s.top(); \
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
			result = two - one;
			s.push(result);
			break;
		case multiply:
			get();
			result = one * two;
			s.push(result);
			break;
		case divide:
			get();
			result = two / one;
			s.push(result);
			break;
		case power:
			get();
			result = pow(two, one);
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
			while (EQ[i2] >= '0' && EQ[i2] <= '9' || EQ[i2] == '.') {
				num.push_back(EQ[i2]);
				i2++;
				index++;
			}
			index--;
			s.push(atof(num.c_str()));
			break;
		}
	}
	return s.top();
}

int precedence(char c) {
	switch (c) {
	case open_p: return -1;
	case add:
	case subtract: return 0;
	case multiply:
	case divide: return 1;
	case power: return 2;
	}
}

void in(istream& in, vector<op>& EQ) {
	char buf = 0;
	int  pos;
	stack<char> s;
	queue<char> q;
	while (!in.eof()) {
		in >> buf;
		if (in.eof()) break;
		switch (buf) {
		case open_p:
			s.push(buf);
			break;
		case close_p:
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
			while (s.size() && precedence(s.top()) == precedence(buf)) {
				q.push(s.top());
				s.pop();
			}
			while (q.size()) {
				s.push(q.front());
				q.pop();
			}
			while (s.size() && precedence(s.top()) > precedence(buf)) {
				EQ.push_back(s.top());
				s.pop();
			}
			s.push(buf);
			break;
		default:
			if (buf >= '0' && buf <= '9' || buf == '.') {
				EQ.push_back(buf);
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

		for (char c : eq)
			cout << c;
		cout << endl;

		float result = eval(eq, x, y);
		cout << x << " " << y << " " << result << endl;

		eq.clear();
	}

	system("pause");
	return 0;
}