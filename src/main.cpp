#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include <vector>

using namespace std;

ifstream fin("in.txt");

float xmin, xmax, ymin, ymax, zmin, zmax, x, y;
string eq;
vector<char> post;

float eval() {
	stack<float> s;
	float one, two, result;
	for (char c : post) {
		switch (c) {
		case '+':
			one = s.top();
			s.pop();
			two = s.top();
			s.pop();
			result = one + two;
			s.push(result);
			break;
		case '-': 
			one = s.top();
			s.pop();
			two = s.top();
			s.pop();
			result = one - two;
			s.push(result);
			break;
		case '*':
			one = s.top();
			s.pop();
			two = s.top();
			s.pop();
			result = one * two;
			s.push(result);
			break;
		case '/': 
			one = s.top();
			s.pop();
			two = s.top();
			s.pop();
			result = one / two;
			s.push(result);
			break;
		case '^': 
			one = s.top();
			s.pop();
			two = s.top();
			s.pop();
			result = pow(one,two);
			s.push(result);
			break;
		case 'x':
			s.push(x);
			break;
		case 'y':
			s.push(y);
			break;
		}
	}
	return s.top();
}

int precedence(char c) {
	switch (c) {
	case '(': return -1;
	case '+':
	case '-': return 0;
	case '*':
	case '/': return 1;
	case '^': return 2;
	}
}

void in() {
	fin >> xmin >> xmax >> ymin >> ymax >> zmin >> zmax >> x >> y;

	char buf;
	int  pos;
	stack<char> s;
	while (fin.good()) {
		fin >> buf;
		if (!fin.good()) break;
		switch (buf) {
		case '(':
			s.push(buf);
			break;
		case ')':
			do {
				buf = s.top();
				if(buf != '(')
					post.push_back(buf);
				s.pop();
			} while (buf != '(');
			break;
		case '*':
		case '/':
		case '+':
		case '-':
		case '^':
			while (s.size() && precedence(s.top()) > precedence(buf)) {
				post.push_back(s.top());
				s.pop();
			}
			s.push(buf);
			break;
		default:
			post.push_back(buf);
			break;
		}
	}
	while (s.size()) {
		post.push_back(s.top());
		s.pop();
	}
}

int main(int argc, char** args) {
	in();
	for (char c : post)
		cout << c;
	cout << endl;

	float result = eval();
	cout << x << " " << y << " " << result << endl;

	system("pause");
	return 0;
}