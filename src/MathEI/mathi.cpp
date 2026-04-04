#include "mathi.h"
#include "parser.h"
#include <functional>
#include <stack>

bool global_rule_state = true;

template<typename _Ty>
inline static bool is_elem_exist(const std::vector<_Ty>& _Vector, size_t _Offset) {
	return _Vector.size() > _Offset;
}

static bool expr_bin_op_rule(std::vector<Token>&, size_t);
static bool expr_enum_rule(std::vector<Token>&, size_t);
static bool expr_un_op_rule(std::vector<Token>&, size_t);
static bool expr_par_rule(std::vector<Token>&, size_t);
static bool expr_var_rule(std::vector<Token>&, size_t);
static bool expr_num_const_rule(std::vector<Token>&, size_t);
static bool expr_func_decl_rule(std::vector<Token>&, size_t);
// static bool expr_cond_rule(std::vector<Token>&, size_t);

struct SyntaxErrorInfo {
	std::string info;
	size_t start, end;
};

SyntaxErrorInfo syntax_error_info;

void set_syntax_error(const SyntaxErrorInfo& _Error) {
	if (!global_rule_state)
		return;

	global_rule_state = false;
	syntax_error_info = _Error;
}

void print_syntax_error(const std::string& _Src, const SyntaxErrorInfo& _ErrorInfo) {
	std::cout
		<< "\nmath: syntax error: " << _ErrorInfo.info << '\n'
		<< "math: " << _Src << '\n'
		<< std::string(_ErrorInfo.start + 6, ' ')
		<< std::string(_ErrorInfo.end - _ErrorInfo.start, '^')
		<< "\n\n";
}

static bool expr_rule(std::vector<Token>& _Tokens, size_t _Offset) {
	return global_rule_state && 
		(expr_un_op_rule(_Tokens, _Offset)
		|| expr_bin_op_rule(_Tokens, _Offset)
		|| expr_func_decl_rule(_Tokens, _Offset)
		|| expr_par_rule(_Tokens, _Offset)
		|| expr_enum_rule(_Tokens, _Offset)
		|| expr_var_rule(_Tokens, _Offset)
		|| expr_num_const_rule(_Tokens, _Offset));
}

static bool expr_var_rule(std::vector<Token>& _Tokens, size_t _Offset) {
	bool current_var = _Tokens.at(_Offset).type == expr_var;

	if (!current_var)
		return false;

	if (is_elem_exist(_Tokens, _Offset + 1)) {
		TokenType next_type = _Tokens.at(_Offset + 1).type;

		bool next_expr = (next_type >= expr_num_const && next_type <= var_enum);

		if (next_expr) {
			set_syntax_error({
				"the expression cannot follow the expression",
				_Tokens.at(_Offset).start, 
				_Tokens.at(_Offset + 1).end
			});

			return false;
		}
	}

	return true;
}

static bool expr_un_op_rule(std::vector<Token>& _Tokens, size_t _Offset) {
	if (!is_elem_exist(_Tokens, _Offset + 1))
		return false;

	TokenType current_type = _Tokens.at(_Offset).type;

	bool current_sign = current_type == bin_op_sub || current_type == bool_un_op_not;

	if (!current_sign)
		return false;

	bool next_expr = expr_rule(_Tokens, _Offset + 1);

	if (!next_expr) {
		set_syntax_error({
			"the expression is expected after the unary operator",
			_Tokens.at(_Offset).start, 
			_Tokens.at(_Offset + 1).end
		});

		return false;
	}

	_Tokens.erase(std::next(_Tokens.begin(), _Offset));
	_Tokens.at(_Offset).type = expr_un_op;

	return true;
}

static bool expr_num_const_rule(std::vector<Token>& _Tokens, size_t _Offset) {
	bool current_num = _Tokens.at(_Offset).type == expr_num_const;

	if (!current_num)
		return false;

	if (is_elem_exist(_Tokens, _Offset + 1)) {
		TokenType next_type = _Tokens.at(_Offset + 1).type;

		bool next_expr = (next_type >= expr_num_const && next_type <= var_enum) || next_type == sym_lpar;

		if (next_expr) {
			set_syntax_error({
				"an expression cannot go after a constant",
				_Tokens.at(_Offset).start, 
				_Tokens.at(_Offset + 1).end
			});

			return false;
		}
	}

	return true;
}

static bool expr_bin_op_rule(std::vector<Token>& _Tokens, size_t _Offset) {
	TokenType type0 = _Tokens.at(_Offset).type;
	
	if (type0 >= expr_num_const && type0 <= var_enum) {
		if (!is_elem_exist(_Tokens, _Offset + 1))
			return false;

		TokenType type1 = _Tokens.at(_Offset + 1).type;

		if (type1 >= bin_op_pow && type1 <= bool_bin_op_or) {
			if (type1 == bin_op_assign && type0 != expr_var) {
				set_syntax_error({
					"the '=' operator can only go after the name or function declaration",
					_Tokens.at(_Offset).start,
					_Tokens.at(_Offset + 1).end
				});

				return false;
			}

			if (!is_elem_exist(_Tokens, _Offset + 2)) {
				set_syntax_error({
					"after '" + _Tokens.at(_Offset + 1).value + "' the expression is expected",
					_Tokens.at(_Offset + 1).start, 
					_Tokens.at(_Offset + 1).end
				});

				return false;
			}

			bool next_expr = expr_rule(_Tokens, _Offset + 2);

			if (_Tokens.at(_Offset + 2).type == stmt_func_decl) {
				set_syntax_error({
					"after '" + _Tokens.at(_Offset + 2).value + "' the expression is expected",
					_Tokens.at(_Offset + 1).start, 
					_Tokens.at(_Offset + 2).end
				});

				return false;
			}

			if (!next_expr) {
				set_syntax_error({
					"the expression is expected",
					_Tokens.at(_Offset + 1).start, 
					_Tokens.at(_Offset + 2).end
				});

				return false;
			}

			_Tokens.erase(std::next(_Tokens.begin(), _Offset));
			_Tokens.erase(std::next(_Tokens.begin(), _Offset));
			_Tokens.at(_Offset).type = expr_bin_op;

			return true;
		}
	}

	return false;
}

static bool expr_enum_rule(std::vector<Token>& _Tokens, size_t _Offset) {
	TokenType type0 = _Tokens.at(_Offset).type;
	
	if (type0 >= expr_num_const && type0 <= var_enum && is_elem_exist(_Tokens, _Offset + 1)) {
		TokenType type1 = _Tokens.at(_Offset + 1).type;

		if (type1 == sym_comma) {
			if (!is_elem_exist(_Tokens, _Offset + 2)) {
				set_syntax_error({
					"the expression is expected",
					_Tokens.at(_Offset + 1).start,
					_Tokens.at(_Offset + 1).end
				});

				return false;
			}

			bool next_expr = expr_rule(_Tokens, _Offset + 2);

			if (!next_expr) {
				set_syntax_error({
					"the expression is expected",
					_Tokens.at(_Offset + 1).start,
					_Tokens.at(_Offset + 2).end
				});

				return false;
			}

			if (type0 == expr_var && _Tokens.at(_Offset + 2).type == expr_var)
				_Tokens.at(_Offset).type = expr_var;
			else 
				_Tokens.at(_Offset).type = expr_enum;

			_Tokens.erase(std::next(_Tokens.begin(), _Offset));
			_Tokens.erase(std::next(_Tokens.begin(), _Offset));

			return true;
		}
	}

	return false;
}

static bool expr_par_rule(std::vector<Token>& _Tokens, size_t _Offset) {
	TokenType type0 = _Tokens.at(_Offset).type;

	if (type0 != sym_lpar)
		return false;

	if (!is_elem_exist(_Tokens, _Offset + 1)) {
		set_syntax_error({
			"after '(' the expression is expected",
			_Tokens.at(_Offset).start,
			_Tokens.at(_Offset).end
		});

		return false;
	}

	bool next_expr;
	size_t current_size;

	do {
		current_size = _Tokens.size();
		next_expr = expr_rule(_Tokens, _Offset + 1);

		if (!next_expr && (current_size != _Tokens.size())) {
			set_syntax_error({
				"after '(' the expression is expected",
				_Tokens.at(_Offset).start,
				_Tokens.at(_Offset + 1).end
			});

			return false;
		}

	} while (current_size != _Tokens.size());

	if (!is_elem_exist(_Tokens, _Offset + 2)) {
		set_syntax_error({
			"after expression the ')' is expected",
			_Tokens.at(_Offset + 1).start,
			_Tokens.at(_Offset + 1).end
		});

		return false;
	}

	TokenType type2 = _Tokens.at(_Offset + 2).type;

	if (type2 != sym_rpar) {
		set_syntax_error({
			"after expression the ')' is expected",
			_Tokens.at(_Offset + 1).start,
			_Tokens.at(_Offset + 2).end
		});

		return false;
	}

	_Tokens.erase(std::next(_Tokens.begin(), _Offset));
	_Tokens.erase(std::next(_Tokens.begin(), _Offset));
	_Tokens.at(_Offset).type = expr_par;

	return true;
}

static bool expr_func_decl_rule(std::vector<Token>& _Tokens, size_t _Offset) {
	if (_Tokens.at(_Offset).type != expr_var)
		return false;

	if (!is_elem_exist(_Tokens, _Offset + 1))
		return false;

	if (_Tokens.at(_Offset + 1).type != sym_lpar)
		return false;

	if (!is_elem_exist(_Tokens, _Offset + 2)) {
		set_syntax_error({
			"after '(' the expected is expected",
			_Tokens.at(_Offset + 1).start, 
			_Tokens.at(_Offset + 1).end
		});

		return false;
	}

	bool next_expr;
	size_t current_size;

	do {
		current_size = _Tokens.size();
		next_expr = expr_rule(_Tokens, _Offset + 2);

		if (!next_expr && (current_size != _Tokens.size())) {
			set_syntax_error({
				"after '(' the expression is expected",
				_Tokens.at(_Offset + 1).start,
				_Tokens.at(_Offset + 2).end
			});

			return false;
		}

	} while (current_size != _Tokens.size());

	if (!is_elem_exist(_Tokens, _Offset + 3)) {
		set_syntax_error({
			"after expression the ')' is expected",
			_Tokens.at(_Offset + 2).start,
			_Tokens.at(_Offset + 2).end
		});

		return false;
	}

	bool next_sym_rpar = _Tokens.at(_Offset + 3).type == sym_rpar;

	if (!next_sym_rpar) {
		set_syntax_error({
			"after expression the ')' is expected",
			_Tokens.at(_Offset + 2).start,
			_Tokens.at(_Offset + 3).end
		});

		return false;
	}

	TokenType expr_type = _Tokens.at(_Offset + 2).type;

	bool next_assign_op = (_Tokens.size() > _Offset + 5) && (_Tokens.at(_Offset + 4).type == bin_op_assign);

	if ((expr_type == var_enum || expr_type == expr_var) && next_assign_op) {
		next_expr = expr_rule(_Tokens, _Offset + 5);

		if (!next_expr || (_Tokens.at(_Offset + 5).type == stmt_func_decl)) {
			set_syntax_error({
				"after '=' the expression is expected",
				_Tokens.at(_Offset + 4).start, 
				_Tokens.at(_Offset + 5).end
			});

			return false;
		}

		_Tokens.erase(std::next(_Tokens.begin(), _Offset));
		_Tokens.erase(std::next(_Tokens.begin(), _Offset));
		_Tokens.erase(std::next(_Tokens.begin(), _Offset));
		_Tokens.erase(std::next(_Tokens.begin(), _Offset));
		_Tokens.erase(std::next(_Tokens.begin(), _Offset));
		_Tokens.at(_Offset).type = stmt_func_decl;
	}
	else {		
		_Tokens.erase(std::next(_Tokens.begin(), _Offset));
		_Tokens.erase(std::next(_Tokens.begin(), _Offset));
		_Tokens.erase(std::next(_Tokens.begin(), _Offset));
		_Tokens.at(_Offset).type = expr_func_call;
	}

	return true;
}

/*static bool expr_cond_rule(std::vector<Token>& _Tokens, size_t _Offset) {
	if (_Tokens.size() <= _Offset + 2)
		return false;

	TokenType current_type = _Tokens.at(_Offset).type;

	bool current_expr = (current_type >= expr_num_const && current_type <= var_enum);

	if (!current_expr)
		return false;

	if (_Tokens.at(_Offset + 1).type != sym_if)
		return false;

_if_cycle:
	cond_stack.push(true);
	bool next_expr = expr_rule(_Tokens, _Offset + 2);
	cond_stack.pop();

	__debugbreak();

	bool next_sym_comma = (_Tokens.size() > _Offset + 3) && (_Tokens.at(_Offset + 3).type == sym_comma);

	__debugbreak();

	if (!next_sym_comma) {
		std::cout << "The ',' is expected after 'if': ["
			<< _Tokens.at(_Offset + 3).start << ", "
			<< _Tokens.at(_Offset + 3).end << "]\n";

		return false;
	}

	next_expr = (_Tokens.size() > _Offset + 4) && expr_rule(_Tokens, _Offset + 4);

	if (!next_expr) {
		std::cout << "The expr is expected after ',': ["
			<< _Tokens.at(_Offset + 4).start << ", "
			<< _Tokens.at(_Offset + 4).end << "]\n";

		return false;
	}

	if (_Tokens.size() <= _Offset + 5) {
		std::cout << "The 'if'/'else' is expected after expr: ["
			<< _Tokens.at(_Offset + 4).start << ", "
			<< _Tokens.at(_Offset + 4).end << "]\n";

		return false;
	}

	TokenType if_else_type = _Tokens.at(_Offset + 5).type;

	if (if_else_type == sym_if) {
		goto _if_cycle;
	}
	else if (if_else_type == sym_else) {

	}
	else {
		std::cout << "The 'if'/'else' is expected after expr: ["
			<< _Tokens.at(_Offset + 4).start << ", "
			<< _Tokens.at(_Offset + 4).end << "]\n";

		return false;
	}
}*/

static bool rule(std::vector<Token>& _Tokens) {
	do {
		bool res = expr_rule(_Tokens, 0);

		if (!res)
			return false;

	} while (_Tokens.size() != 1);

	return true;
}

double MathI::eval(const std::string& _Str) {
	global_rule_state = true;
	const size_t length = _Str.length();

	for (size_t i = 0, j = std::min(_Str.find(';', i), length); i < length; i = j + 1, j = std::min(_Str.find(';', j + 1), length)) {
		Tokenizer tokens;

		std::string sub_str = _Str.substr(i, j - i);
		if (!tokens.parse(sub_str)) {
			break;
		}

		std::vector<Token> token_array{ tokens.m_Tokens };

		bool current_expr = rule(token_array);

		if (!current_expr) {
			print_syntax_error(sub_str, syntax_error_info);

			return 0.0;
		}
	}

	return 0.0;
};