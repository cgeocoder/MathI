#pragma once 

#ifndef __MATH_EXPRESSION_INTERPRETER_H__
#define __MATH_EXPRESSION_INTERPRETER_H__

#include <string>
#include "tokens.h"

class MathIObject;

class MathIConstant {
public:
	inline MathIConstant(double _Val) : value{ _Val } {}
	double value;
};

class MathIVariable {
public:
	inline MathIVariable() : initialized{ false }, value{ 0.0 } {}
	bool initialized;
	double value;
};

struct MathIParamList {
	std::vector<MathIObject*> params;
};

struct MathiIFunction {
	size_t number_of_params;
	std::vector<Opcode> opcode;
};

class MathIObject {
public:
	bool callable;
	bool constant;
	std::string name;
	void* val_ptr;

	static inline MathIObject make_const(double _Value) {
		return MathIObject{
			false,
			true,
			"",
			static_cast<void*>(new MathIConstant(_Value))
		};
	}

	static inline MathIObject make_named_object(const std::string& _Name) {
		return MathIObject{
			false,
			false,
			_Name,
			nullptr
		};
	}

	static inline MathIObject make_func(const std::string& _Name, MathiIFunction* _Func) {
		return MathIObject{
			true,
			false,
			_Name,
			static_cast<void*>(_Func)
		};
	}
};

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
	size_t stack[max_stack_length] = { 0 };
	size_t stack_counter = 0;

	size_t offset_stack[max_stack_length] = { 0 };
	size_t offset_stack_counter = 0;

	struct Function {
		std::string name;
		std::vector<std::string> arg_name;
		double* args = nullptr;
		std::vector<Opcode> opcode;
		size_t addr = -1;
	};

	std::vector<Opcode> opcode;
	std::vector<MathIObject> m_Objects;

	size_t get_object_index(const MathIObject& _Obj);
	size_t get_object_index_by_name(const std::string& _Name);

	void generate_ast(const std::vector<Token>& _Tokens);

	void r_gen_opcode(std::vector<Opcode>&, AST*, const std::vector<std::string>& params = std::vector<std::string>());
	void gen_executable();

	void debug_print_opcode(const std::vector<Opcode>& _Opcode);
	size_t execute(std::vector<Opcode>& _Opcode);

public:
	MathI();

	double eval(const std::string& _Str);
};

#endif // !__MATH_EXPRESSION_INTERPRETER_H__