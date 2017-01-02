
#pragma once

#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

const float val_e = 2.7182818284590452353602874713527f;
const float val_pi = 3.141592653589793238462643383279f;

typedef int op;

float eval(const vector<op>& EQ, vector<tuple<char, float>> vars);
bool in(string str, vector<op>& EQ);
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
	op_neg = 0,
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
	op_sec = 1016,
	op_csc = 1017,
	op_cot = 1018,
	op_asec = 1019,
	op_acsc = 1020,
	op_acot = 1021,

	var = 9999,

	// constants
	const_pi = 'P',
	const_e = 'E'
};