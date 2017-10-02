
bool isop(wchar_t c) {
	return c == open_p || c == add || c == subtract || c == op_neg ||
		   c == multiply || c == modulo || c == divide || c == power;
}

bool isnonfunc(wchar_t c) {
	return c == close_p || c == add || c == subtract || c == op_neg ||
		   c == multiply || c == modulo || c == divide || c == power;
}

bool num(wchar_t c) {
	return c >= '0' && c <= '9' || c == '.';
}

#define get2() if(s.size() > 1) {two = s.top();s.pop();one = s.top();s.pop();} \
			   else {throw runtime_error("Malformed expression!");}

#define get1() if(s.size()) {one = s.top();s.pop();} \
			   else {throw runtime_error("Malformed expression!");}

float eval(const vector<op>& EQ, vector<pair<wchar_t, float>> vars) {
	if (!EQ.size()) throw runtime_error("Empty expression!");
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
			wchar_t v = EQ[index + 1];
			auto entry = find_if(vars.begin(), vars.end(), [v](const pair<wchar_t, float>& var) -> bool { return var.first == v; });
			if (entry == vars.end()) {
				wstring error = L"Variable '";
				error += v;
				error += L"' not recognized!";
				throw runtime_error(wstring_to_utf8(error));
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
				num.push_back((char)EQ[i2]);
				i2++;
				index++;
			}
			s.push((float)atof(num.c_str()));
			break;
		}
	}
	result = s.top();
	s.pop();
	if (s.size()) throw runtime_error("Malformed expression!");
	return result;
}

int precedence(wchar_t c) {
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

void in(wstring _str, vector<op>& EQ) {

	wstring str;

	for (int ind = 0; ind < (int)_str.length() && _str[ind]; ind++) {
		if (_str[ind] == '-' && (ind == 0 || isop(_str[ind - 1]))) _str[ind] = op_neg;
		if (_str[ind] == ' ') {
			_str.erase(ind, 1);
			ind--;
		}
		str.push_back(_str[ind]);
	}

	wchar_t buf = 0;
	stack<op> s;
	queue<op> q;
	wstringstream in;
	in << str;
	bool added = false, ins = true;
	while (!in.eof()) {
		if (ins) {
			in >> buf;
		}
		if (in.eof()) break;
		switch (buf) {
		case open_p:
			ins = true;
			s.push(buf);
			break;
		case close_p:
			ins = true;
			if (s.size()) {
				wchar_t cur;
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
#ifdef _MSC_VER
				if (in.peek() != WCHAR_MAX && !isnonfunc(in.peek())) {
#else
				if (in.peek() != EOF && !isnonfunc(in.peek())) {
#endif
					wstring func;
					getline(in, func, L'(');
					func.insert(0, 1, buf);
					// Test function name
					{
						if (func == L"sqrt")
							s.push(op_sqrt);
						else if (func == L"sin")
							s.push(op_sin);
						else if (func == L"cos")
							s.push(op_cos);
						else if (func == L"tan")
							s.push(op_tan);
						else if (func == L"asin")
							s.push(op_asin);
						else if (func == L"acos")
							s.push(op_acos);
						else if (func == L"atan")
							s.push(op_atan);
						else if (func == L"abs")
							s.push(op_abs);
						else if (func == L"exp")
							s.push(op_exp);
						else if (func == L"exptwo")
							s.push(op_exptwo);
						else if (func == L"ceil")
							s.push(op_ceil);
						else if (func == L"floor")
							s.push(op_floor);
						else if (func == L"ln")
							s.push(op_ln);
						else if (func == L"log")
							s.push(op_log);
						else if (func == L"log2")
							s.push(op_log2);
						else if (func == L"sec")
							s.push(op_sec);
						else if (func == L"csc")
							s.push(op_csc);
						else if (func == L"cot")
							s.push(op_cot);
						else if (func == L"asec")
							s.push(op_asec);
						else if (func == L"acsc")
							s.push(op_acsc);
						else if (func == L"acot")
							s.push(op_acot);
						else {
							if (!in.good())
								throw runtime_error("Unknown name '" + wstring_to_utf8(func) + "'!");
							else
								throw runtime_error("Unknown function '" + wstring_to_utf8(func) + "()'!");
						}
						if(!in.good()) {
							throw runtime_error("No parenthesis after '" + wstring_to_utf8(func) + "'!");
						}
						if(in.peek() == ')') {
							throw runtime_error("No arguments in '" + wstring_to_utf8(func) + "'!");	
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
			throw runtime_error("Unbalanced parenthesis!");
		}
		EQ.push_back(s.top());
		s.pop();
	}
	if (!EQ.size()) {
		throw runtime_error("Empty expression!");
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
			out << c;
		out << " ";
	}
	out << endl;
}
