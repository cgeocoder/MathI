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
	static constexpr size_t max_stack_length = 512;
	double stack[max_stack_length] = { 0.0 };
	size_t stack_counter = 0;


	struct Object {
		std::string name;
		float init = false;
		double value = 0.0;
	};

	struct Function {
		std::string name;
		std::vector<std::string> arg_name;
		double* args = nullptr;
		std::vector<Opcode> opcode;
		size_t addr;
	};

	std::vector<Object> symbol_table;
	std::vector<Opcode> opcode;
	std::vector<Function> functions;

	int get_symbol_id(const std::string& _Name);
	void generate_ast(const std::vector<Token>& _Tokens);

	void r_gen_opcode(std::vector<Opcode>&, AST*, const Function& func = Function());
	void gen_executable();

	void debug_print_opcode(const std::vector<Opcode>& _Opcode);
	double execute(std::vector<Opcode>& _Opcode);

public:
	double eval(const std::string& _Str);
};

#endif // !__MATH_EXPRESSION_INTERPRETER_H__