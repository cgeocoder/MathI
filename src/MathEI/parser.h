#pragma once

#ifndef __PARSER_H__
#define __PARSER_H__

#include "tokens.h"
#include <stack>

struct _ParserMetadata {
	std::string_view src;
	size_t par_depth;
};

template<class _Ty>
inline bool is_elem_exist(const std::vector<_Ty>& _Vector, size_t _Offset) {
	return _Vector.size() > _Offset;
}

void rule(std::vector<Token>&, const std::string_view&);
bool expr_rule(std::vector<Token>&, size_t, _ParserMetadata&);
bool expr_bin_op_rule(std::vector<Token>&, size_t, _ParserMetadata&);
bool expr_enum_rule(std::vector<Token>&, size_t, _ParserMetadata& c);
bool expr_un_op_rule(std::vector<Token>&, size_t, _ParserMetadata&);
bool expr_par_rule(std::vector<Token>&, size_t, _ParserMetadata&);
bool expr_var_rule(std::vector<Token>&, size_t, _ParserMetadata&);
bool expr_num_const_rule(std::vector<Token>&, size_t, _ParserMetadata&);
bool expr_func_decl_rule(std::vector<Token>&, size_t, _ParserMetadata&);

#endif // !__PARSER_H__