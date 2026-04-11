#pragma once

#ifndef __PARSER_H__
#define __PARSER_H__

#include "tokens.h"
#include <stack>

struct SyntaxErrorInfo {
	std::string info;
	size_t start = 0, end = 0;
};

inline bool is_elem_exist(const std::vector<Token>& _Vector, size_t _Offset) {
	return _Vector.size() > _Offset;
}

bool rule(std::vector<Token>&);
bool expr_rule(std::vector<Token>&, size_t);
bool expr_bin_op_rule(std::vector<Token>&, size_t);
bool expr_enum_rule(std::vector<Token>&, size_t);
bool expr_un_op_rule(std::vector<Token>&, size_t);
bool expr_par_rule(std::vector<Token>&, size_t);
bool expr_var_rule(std::vector<Token>&, size_t);
bool expr_num_const_rule(std::vector<Token>&, size_t);
bool expr_func_decl_rule(std::vector<Token>&, size_t);
bool expr_cond_rule(std::vector<Token>&, size_t);

void set_syntax_error(const SyntaxErrorInfo&);
void print_syntax_error(const std::string&, const SyntaxErrorInfo&);

inline bool global_rule_state = true;
inline SyntaxErrorInfo syntax_error_info;
inline std::stack<bool> is_condition_stack;


#endif // !__PARSER_H__