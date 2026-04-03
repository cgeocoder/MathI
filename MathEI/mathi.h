#pragma once 

#ifndef __MATH_EXPRESSION_INTERPRETER_H__
#define __MATH_EXPRESSION_INTERPRETER_H__

#include <string>
#include "tokens.h"

class MathI {
private:
	// GrammarRule rules;

public:
	double eval(const std::string& _Str);
};

#endif // !__MATH_EXPRESSION_INTERPRETER_H__