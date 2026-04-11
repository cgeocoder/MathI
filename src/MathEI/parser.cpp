#include "parser.h"


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
			|| ((!is_condition_stack.top()) && expr_cond_rule(_Tokens, _Offset))
			|| expr_par_rule(_Tokens, _Offset)
			|| ((!is_condition_stack.top()) && expr_enum_rule(_Tokens, _Offset))
			|| expr_var_rule(_Tokens, _Offset)
			|| expr_num_const_rule(_Tokens, _Offset));
}

bool expr_var_rule(std::vector<Token>& _Tokens, size_t _Offset) {
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

		if ((next_type == sym_else) && !is_condition_stack.top()) {
			set_syntax_error({
				"the 'else' cannot follow the expression",
				_Tokens.at(_Offset).start,
				_Tokens.at(_Offset + 1).end
				});

			return false;
		}
	}

	return true;
}

bool expr_un_op_rule(std::vector<Token>& _Tokens, size_t _Offset) {
	if (!is_elem_exist(_Tokens, _Offset + 1))
		return false;

	TokenType current_type = _Tokens.at(_Offset).type;

	bool current_sign = current_type == un_op_sub || current_type == bool_un_op_not;

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

bool expr_num_const_rule(std::vector<Token>& _Tokens, size_t _Offset) {
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

		if ((next_type == sym_else) && !is_condition_stack.top()) {
			set_syntax_error({
				"the 'else' cannot follow the expression",
				_Tokens.at(_Offset).start,
				_Tokens.at(_Offset + 1).end
				});

			return false;
		}
	}

	return true;
}

bool expr_bin_op_rule(std::vector<Token>& _Tokens, size_t _Offset) {
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

			if ((type1 == bin_op_pow) && (_Tokens.at(_Offset + 2).type == expr_un_op)) {
				set_syntax_error({
					"not unary expression is expected",
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

bool expr_enum_rule(std::vector<Token>& _Tokens, size_t _Offset) {
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

bool expr_par_rule(std::vector<Token>& _Tokens, size_t _Offset) {
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

		is_condition_stack.push(false);
		next_expr = expr_rule(_Tokens, _Offset + 1);
		is_condition_stack.pop();

		if (!next_expr && (current_size != _Tokens.size())) {
			set_syntax_error({
				"after '(' the expression is expected",
				_Tokens.at(_Offset).start,
				_Tokens.at(_Offset + 1).end
				});

			return false;
		}

	} while (current_size != _Tokens.size());

	if (_Tokens.at(_Offset + 1).type == stmt_func_decl) {
		set_syntax_error({
			"after '(' the expression is expected",
			_Tokens.at(_Offset).start,
			_Tokens.at(_Offset + 1).end
			});

		return false;
	}

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

bool expr_func_decl_rule(std::vector<Token>& _Tokens, size_t _Offset) {
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

		is_condition_stack.push(false);
		next_expr = expr_rule(_Tokens, _Offset + 2);
		is_condition_stack.pop();

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
		is_condition_stack.push(false);
		next_expr = expr_rule(_Tokens, _Offset + 5);
		is_condition_stack.pop();

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

bool expr_cond_rule(std::vector<Token>& _Tokens, size_t _Offset) {
	size_t local_offset = 0;

	TokenType current_type = _Tokens.at(_Offset).type;

	bool current_expr = (current_type >= expr_num_const && current_type <= var_enum);

	if (!current_expr)
		return false;

	if (!is_elem_exist(_Tokens, _Offset + 1) || (_Tokens.at(_Offset + 1).type != sym_if))
		return false;

	if (!is_elem_exist(_Tokens, _Offset + 2)) {
		set_syntax_error({
			"after 'if' the expression is expected",
			_Tokens.at(_Offset + 1).start,
			_Tokens.at(_Offset + 1).end
			});

		return false;
	}

_if_cycle:

	bool next_expr;
	size_t current_size;

	do {
		current_size = _Tokens.size();

		is_condition_stack.push(true);
		next_expr = expr_rule(_Tokens, _Offset + 2 + local_offset);
		is_condition_stack.pop();

		if (!next_expr && (current_size != _Tokens.size())) {
			set_syntax_error({
				"after 'if' the expression is expected",
				_Tokens.at(_Offset + 1).start,
				_Tokens.at(_Offset + 2).end
				});

			return false;
		}

	} while (current_size != _Tokens.size());

	if (!is_elem_exist(_Tokens, _Offset + 3 + local_offset)) {
		set_syntax_error({
			"after expression the ',' is expected",
			_Tokens.at(_Offset + 2 + local_offset).start,
			_Tokens.at(_Offset + 2 + local_offset).end
			});

		return false;
	}

	bool next_sym_comma = (_Tokens.at(_Offset + 3 + local_offset).type == sym_comma);

	if (!next_sym_comma) {
		set_syntax_error({
			"after expression the ',' is expected",
			_Tokens.at(_Offset + 2 + local_offset).start,
			_Tokens.at(_Offset + 3 + local_offset).end
			});

		return false;
	}

	if (!is_elem_exist(_Tokens, _Offset + 4 + local_offset)) {
		set_syntax_error({
			"after ',' the expression is expected",
			_Tokens.at(_Offset + 3 + local_offset).start,
			_Tokens.at(_Offset + 3 + local_offset).end
			});

		return false;
	}

	do {
		current_size = _Tokens.size();

		is_condition_stack.push(true);
		next_expr = expr_rule(_Tokens, _Offset + 4 + local_offset);
		is_condition_stack.pop();

		if (!next_expr && (current_size != _Tokens.size())) {
			set_syntax_error({
				"after ',' the expression is expected",
				_Tokens.at(_Offset + 3).start,
				_Tokens.at(_Offset + 4).end
				});

			return false;
		}

	} while (current_size != _Tokens.size());

	// Next may be 'if' or 'else'

	if (!is_elem_exist(_Tokens, _Offset + 5 + local_offset)) {
		set_syntax_error({
			"after expression the 'if' or 'else' is expected",
			_Tokens.at(_Offset + 4 + local_offset).start,
			_Tokens.at(_Offset + 4 + local_offset).end
			});

		return false;
	}

	TokenType next_token_type = _Tokens.at(_Offset + 5 + local_offset).type;

	if (next_token_type == sym_if) {
		local_offset += 4;

		goto _if_cycle;
	}
	else if (next_token_type == sym_else) {
		_Tokens.erase(
			std::next(_Tokens.begin(), _Offset),
			std::next(_Tokens.begin(), _Offset + 5 + local_offset)
		);

		_Tokens.at(_Offset).type = expr_cond;

		return true;
	}
	else {
		set_syntax_error({
			"after expression the 'if' or 'else' is expected",
			_Tokens.at(_Offset + 4 + local_offset).start,
			_Tokens.at(_Offset + 4 + local_offset).end
		});

		return false;
	}

	return false;
}

bool rule(std::vector<Token>& _Tokens) {
	do {
		bool res = expr_rule(_Tokens, 0);

		if (!res)
			return false;

	} while (_Tokens.size() != 1);

	return true;
}
