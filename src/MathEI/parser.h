#pragma once

#ifndef __PARSER_H__
#define __PARSER_H__

#include "tokens.h"
#include <stack>

inline bool is_elem_exist(const std::vector<Token>& _Vector, size_t _Offset) {
	return _Vector.size() > _Offset;
}

void rule(std::vector<Token>&, const std::string_view& _Src);
bool expr_rule(std::vector<Token>&, size_t, const std::string_view& _Src);
bool expr_bin_op_rule(std::vector<Token>&, size_t, const std::string_view& _Src);
bool expr_enum_rule(std::vector<Token>&, size_t, const std::string_view& _Src);
bool expr_un_op_rule(std::vector<Token>&, size_t, const std::string_view& _Src);
bool expr_par_rule(std::vector<Token>&, size_t, const std::string_view& _Src);
bool expr_var_rule(std::vector<Token>&, size_t, const std::string_view& _Src);
bool expr_num_const_rule(std::vector<Token>&, size_t, const std::string_view& _Src);
bool expr_func_decl_rule(std::vector<Token>&, size_t, const std::string_view& _Src);

#endif // !__PARSER_H__