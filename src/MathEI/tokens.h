#pragma once

#ifndef __TOKENS_H__
#define __TOKENS_H__

#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include "errors.h"

enum TokenType {
	nothing,
	begin = nothing,
	sym_eos,		// end of string
	sym_comma,
	sym_semicolon,
	sym_lpar,
	sym_rpar,

	// ## Binary Operators

	bin_op_pow,
	bin_op_mul,
	bin_op_div,
	bin_op_add,
	bin_op_sub,
	bin_op_assign,

	bool_bin_op_less,
	bool_bin_op_more,
	bool_bin_op_less_eq,
	bool_bin_op_more_eq,
	bool_bin_op_eq,
	bool_bin_op_not_eq,
	bool_bin_op_and,
	bool_bin_op_or,

	// bin_op, 

	bool_un_op_not,

	un_op_sub,

	// ## MathEI Expression

	expr_num_const,
	expr_var,
	expr_un_op,
	expr_bin_op,
	expr_par,
	expr_func_call,
	expr_cond,
	expr_enum,
	var_enum, // for func decl

	expr,

	// # Unit of Statements

	stmt_func_decl,

	stmt,

	final_instruction,
};

enum class Opcode : size_t {
	/* 
	* Puts the address of the MathIObject
		at the top of the stack 
	*
	* Syntax: 
		push [index: void* -> size_t -> Opcode]
	*/
	push,

	push_const,

	/*
	* Load From Stack
	* Puts the address of the MathIObject at the top of the stack 
		adding the offset from the top of the offset_stack

	*	Equals:
			push [index + offset_stack.top]
	*/
	lfs,

	bin_op,
	bo_pow = bin_op_pow,
	bo_mul = bin_op_mul,
	bo_div = bin_op_div,
	bo_add = bin_op_add,
	bo_sub = bin_op_sub,
	bo_assign = bin_op_assign,
	bo_less = bool_bin_op_less,
	bo_less_eq = bool_bin_op_less_eq,
	bo_more = bool_bin_op_more,
	bo_more_eq = bool_bin_op_more_eq,
	bo_eq = bool_bin_op_eq,
	bo_not_eq = bool_bin_op_not_eq,
	bo_and = bool_bin_op_and,
	bo_or = bool_bin_op_or,

	un_op,
	uo_neg,
	uo_not,

	/*
	* Calls the function
	* 1) puts the offset at the offset_stack
	*	offset_stack.top = stack.ptr - param_count
	* 
	* 2) call function
	* 3) clears offset_stack and stack after itself
	*/
	call,

	// call_builtin [address]
	call_builtin,

	// store [address]
	// store a value in a program stack
	store,

	make_func,

	// stop program
	halt,
};

class Token {
public:
	inline Token(){}

	inline Token(
		TokenType _Type, 
		const std::string& _Value,
		size_t _Start,
		size_t _End
	)
		: type{ _Type }, value{ _Value }, start{ _Start }, end{ _End } {}

	TokenType type;
	std::string value;
	size_t start, end;

	inline Token(const Token& _Copy) noexcept
		: type{ _Copy.type }, value{ _Copy.value }, start{ _Copy.start }, end{ _Copy.end } {}
	
	inline Token(Token&& _Move) noexcept
		: type{ _Move.type }, value{ _Move.value }, start{ _Move.start }, end{ _Move.end } 
	{
		_Move.start = _Move.end = 0;
		_Move.value.clear();
		_Move.type = TokenType::nothing;
	}

	inline Token& operator=(const Token& _Copy) noexcept {
		type = _Copy.type;
		value = _Copy.value;
		start = _Copy.start;
		end = _Copy.end;
		return *this;
	}

	inline Token& operator=(Token&& _Move) noexcept	{
		type = _Move.type;
		value = _Move.value;
		start = _Move.start;
		end = _Move.end;

		_Move.start = _Move.end = 0;
		_Move.value.clear();
		_Move.type = TokenType::nothing;
		return *this;
	}

	~Token() = default;
};

class Tokenizer {
	friend class MathI;
	friend class Parser;

public:
	static void parse(
		const std::string& _Str, 
		std::vector<Token>& _Tokens,
		MathIErrorManager& _ErrorManager
	);
};

#endif // !__TOKENS_H__