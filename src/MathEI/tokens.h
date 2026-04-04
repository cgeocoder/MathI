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

	// bool_un_op,

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
	friend class EvalTree;

private:
	std::vector<Token> m_Tokens;

public:
	Tokenizer();

	bool parse(const std::string& _Str);
};

bool is_expr(TokenType* t);
bool is_stmt(TokenType* t);
bool is_un_op(TokenType* t);
bool is_bin_op(TokenType* t);
bool is_expr_par(TokenType* t);
bool is_expr_enum(TokenType* t);
bool is_expr_un_op(TokenType* t);
bool is_expr_bin_op(TokenType* t);
bool cond_after_expr(TokenType* t);
bool is_expr_func_call(TokenType* t);
bool cond_before_expr_un_op(TokenType* t);
// bool is_cond_if_part(TokenType* t);
bool is_cond_if_part_enum(TokenType* t);
bool is_cond_else_part(TokenType* t);
bool is_expr_cond(TokenType* t);
bool is_var_enum(TokenType* t);
bool is_stmt_func_decl(TokenType* t);
bool is_final_instruction(TokenType* t);


#endif // !__TOKENS_H__