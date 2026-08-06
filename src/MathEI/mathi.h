#pragma once 

#ifndef __MATH_EXPRESSION_INTERPRETER_H__
#define __MATH_EXPRESSION_INTERPRETER_H__

#include <string>
#include <format>
#include "tokens.h"
#include "errors.h"

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

static std::string get_mathi_object_info(MathIObject* o) {
	if (o == nullptr)
		return "[object is <nullptr>]";

	if (o->callable)
		return std::format("[function '{}' at 0x{:x}]", o->name, (size_t)o->val_ptr).c_str();

	if (o->constant)
		return std::format("[const '{}']", static_cast<MathIConstant*>(o->val_ptr)->value).c_str();

	else {
		auto v = static_cast<MathIVariable*>(o->val_ptr);

		if (v != nullptr && v->initialized)
			return std::format("[var {}={}]", o->name, v->value);
		else {
			return std::format("[var {}=???]", o->name);
		}
	}
}

class MathI {
private:
	MathIErrorManager m_ErrorManager;

	AST* m_AST;
	static constexpr size_t max_stack_length = 512;
	MathIObject* stack[max_stack_length] = { 0 };
	size_t stack_counter = 0;

	size_t offset_stack[max_stack_length] = { 0 };
	size_t offset_stack_counter = 0;

	std::vector<Opcode> opcode;
	std::vector<MathIObject> m_Objects;

	std::string m_CurrentSrc;

	size_t get_object_index(const MathIObject& _Obj);
	size_t get_object_index_by_name(const std::string& _Name);

	void generate_ast(const std::vector<Token>& _Tokens);

	void r_gen_opcode(std::vector<Opcode>&, AST*, const std::vector<std::string>& params);
	void gen_executable();

	void debug_print_opcode(const std::vector<Opcode>& _Opcode);
	MathIObject* execute(std::vector<Opcode>& _Opcode);

	inline void clear_error() { m_ErrorManager.clear(); }

public:
	inline MathI() {}
	double eval(const std::string& _Str);

	inline bool ok() { return m_ErrorManager.ok(); }
	inline std::string get_error() { return m_ErrorManager.get_last_error(); }
};

#endif // !__MATH_EXPRESSION_INTERPRETER_H__