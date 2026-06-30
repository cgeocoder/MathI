#pragma once 

#ifndef __MATH_EXPRESSION_INTERPRETER_H__
#define __MATH_EXPRESSION_INTERPRETER_H__

#include <string>
#include "tokens.h"

class AST {
public:
	inline AST(const Token& _Token) : token{ _Token } {}

	Token token;
	std::vector<AST*> nodes;
};

class MathI {
private:
	AST* m_AST;

	struct Object {
		std::string name;
		float init = false;
		double value = 0.0;
	};

	std::vector<Object> symbol_table;
	std::vector<Opcode> opcode;

	int get_symbol_id(const std::string& _Name);
	void geterate_ast(const std::vector<Token>& _Tokens);

	void r_gen_opcode(std::vector<Opcode>&, AST*);
	void gen_executable();

	void debug_print_opcode();

public:
	double eval(const std::string& _Str);
};

#endif // !__MATH_EXPRESSION_INTERPRETER_H__