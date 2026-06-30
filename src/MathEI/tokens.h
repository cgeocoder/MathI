#pragma once

#ifndef __TOKENS_H__
#define __TOKENS_H__

#include <vector>
#include <string>
#include <memory>
#include <iostream>

enum TokenType {
	nothing,
	begin = nothing,
	sym_eos,		// end of string
	sym_comma,
	sym_semicolon,
	sym_lpar,
	sym_rpar,
	sym_if,
	sym_else,

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

	cond_if_part,
	cond_if_part_enum,
	cond_else_part,

	// ### Unary Operation Expression

	// expr_un_op,

	// # Unit of Statements

	stmt_func_decl,

	stmt,

	final_instruction,
};

enum class Opcode : size_t {
	// push [id]
	// id - index of value
	push,

	// push value
	push_const,

	// pop
	pop,

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

	// call [address]
	// address - a number point to the function
	call,

	// call [address]
	store,

	// clear program stack
	clear_stack,

	// stop program
	halt,
};


class Token {
public:
	inline Token(
		TokenType _Type, 
		const std::string& _Val,
		size_t _Start,
		size_t _End
	)
		: type{ _Type }, value{ _Val }, start{ _Start }, end{ _End } {}

	TokenType type;
	std::string value;
	size_t start, end;
};

class TokenTree {
public:
	inline TokenTree(
		const Token& _Token,
		TokenTree* _Prev
	) : token{ _Token }, prev{ _Prev }, next{ nullptr } {}

	Token token;
	TokenTree* next, *prev;
	std::vector<TokenTree*> nodes;

	inline TokenTree* add_next(const Token& _Token) {
		next = new TokenTree(_Token, nullptr);
		prev = this;

		std::cout << "'" << _Token.value << "'\n";

		return next;
	}

	inline void add_sub(TokenTree* _SubTree) {
		_SubTree->prev = _SubTree->next = nullptr;
		nodes.push_back(_SubTree);
	}

	inline void __cdecl operator delete(void* _Block) {
		// (void)(std::cout << __FUNCSIG__ << '\n');

		if (_Block != nullptr)
			delete _Block;

		_Block = nullptr;
	}

	inline void __cdecl operator delete[](void* _Block) {
		// (void)(std::cout << __FUNCSIG__ << '\n');

		if (_Block != nullptr)
			delete[] _Block;

		_Block = nullptr;
	}

	inline bool is_start(void) const {
		return prev == nullptr;
	}

	inline bool is_end(void) const {
		return next == nullptr;
	}
};

class Tokenizer {
	friend class MathI;
	friend class Parser;

private:
	std::vector<Token> m_Tokens;

public:
	Tokenizer();

	bool parse(const std::string& _Str);
};

#endif // !__TOKENS_H__