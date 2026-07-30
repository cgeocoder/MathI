#include "parser.h"
#include "errors.h"

static bool expr_rule(std::vector<Token>& _Tokens, size_t _Offset, const std::string_view& _Src) {
	return expr_un_op_rule(_Tokens, _Offset, _Src)
			|| expr_bin_op_rule(_Tokens, _Offset, _Src)
			|| expr_func_decl_rule(_Tokens, _Offset, _Src)
			|| expr_par_rule(_Tokens, _Offset, _Src)
			|| expr_enum_rule(_Tokens, _Offset, _Src)
			|| expr_var_rule(_Tokens, _Offset, _Src)
			|| expr_num_const_rule(_Tokens, _Offset, _Src);
}

bool expr_var_rule(std::vector<Token>& _Tokens, size_t _Offset, const std::string_view& _Src) {
	bool current_var = _Tokens.at(_Offset).type == expr_var;

	if (!current_var)
		return false;

	if (is_elem_exist(_Tokens, _Offset + 1)) {
		TokenType next_type = _Tokens.at(_Offset + 1).type;

		bool next_expr = (next_type >= expr_num_const && next_type <= var_enum);

		if (next_expr) {
			throw MathISyntaxError(
				_Src,
				"the expression cannot follow the expression",
				_Tokens.at(_Offset).start,
				_Tokens.at(_Offset + 1).end
			);

			return false;
		}
	}

	return true;
}

bool expr_un_op_rule(std::vector<Token>& _Tokens, size_t _Offset, const std::string_view& _Src) {
	if (!is_elem_exist(_Tokens, _Offset + 1))
		return false;

	TokenType current_type = _Tokens.at(_Offset).type;

	bool current_sign = current_type == un_op_sub || current_type == bool_un_op_not;

	if (!current_sign)
		return false;

	bool next_expr = expr_rule(_Tokens, _Offset + 1, _Src);

	if (!next_expr) {
		throw MathISyntaxError(
			_Src,
			"the expression is expected after the unary operator",
			_Tokens.at(_Offset).start,
			_Tokens.at(_Offset + 1).end
		);

		return false;
	}

	_Tokens.erase(std::next(_Tokens.begin(), _Offset));
	_Tokens.at(_Offset).type = expr_un_op;

	return true;
}

bool expr_num_const_rule(std::vector<Token>& _Tokens, size_t _Offset, const std::string_view& _Src) {
	bool current_num = _Tokens.at(_Offset).type == expr_num_const;

	if (!current_num)
		return false;

	if (is_elem_exist(_Tokens, _Offset + 1)) {
		TokenType next_type = _Tokens.at(_Offset + 1).type;

		bool next_expr = (next_type >= expr_num_const && next_type <= var_enum) || next_type == sym_lpar;

		if (next_expr) {
			throw MathISyntaxError(
				_Src,
				"the expression cannot go after a constant",
				_Tokens.at(_Offset).start,
				_Tokens.at(_Offset + 1).end
			);

			return false;
		}
	}

	return true;
}

bool expr_bin_op_rule(std::vector<Token>& _Tokens, size_t _Offset, const std::string_view& _Src) {
	TokenType type0 = _Tokens.at(_Offset).type;

	if (type0 >= expr_num_const && type0 <= var_enum) {
		if (!is_elem_exist(_Tokens, _Offset + 1))
			return false;

		TokenType type1 = _Tokens.at(_Offset + 1).type;

		if (type1 >= bin_op_pow && type1 <= bool_bin_op_or) {
			if (type1 == bin_op_assign && type0 != expr_var) {
				throw MathISyntaxError(
					_Src,
					"the '=' operator can only go after the name or function declaration",
					_Tokens.at(_Offset).start,
					_Tokens.at(_Offset + 1).end
				);

				return false;
			}

			if (!is_elem_exist(_Tokens, _Offset + 2)) {
				throw MathISyntaxError(
					_Src,
					std::vformat("after '{}' the expression is expected", std::make_format_args(_Tokens.at(_Offset + 1).value)),
					_Tokens.at(_Offset + 1).start,
					_Tokens.at(_Offset + 1).end
				);

				return false;
			}

			bool next_expr = expr_rule(_Tokens, _Offset + 2, _Src);

			if (_Tokens.at(_Offset + 2).type == stmt_func_decl) {
				throw MathISyntaxError(
					_Src,
					std::vformat("after '{}' the expression is expected", std::make_format_args(_Tokens.at(_Offset + 2).value)),
					_Tokens.at(_Offset + 1).start,
					_Tokens.at(_Offset + 2).end
				);

				return false;
			}

			if (!next_expr) {
				throw MathISyntaxError(
					_Src,
					"the expression is expected",
					_Tokens.at(_Offset + 1).start,
					_Tokens.at(_Offset + 2).end
				);

				return false;
			}

			if ((type1 == bin_op_pow) && (_Tokens.at(_Offset + 2).type == expr_un_op)) {
				throw MathISyntaxError(
					_Src,
					"'not' unary expression is expected",
					_Tokens.at(_Offset + 1).start,
					_Tokens.at(_Offset + 2).end
				);

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

bool expr_enum_rule(std::vector<Token>& _Tokens, size_t _Offset, const std::string_view& _Src) {
	TokenType type0 = _Tokens.at(_Offset).type;

	if (type0 >= expr_num_const && type0 <= var_enum && is_elem_exist(_Tokens, _Offset + 1)) {
		TokenType type1 = _Tokens.at(_Offset + 1).type;

		if (type1 == sym_comma) {
			if (!is_elem_exist(_Tokens, _Offset + 2)) {
				throw MathISyntaxError(
					_Src,
					"the expression is expected",
					_Tokens.at(_Offset + 1).start,
					_Tokens.at(_Offset + 1).end
				);

				return false;
			}

			bool next_expr = expr_rule(_Tokens, _Offset + 2, _Src);

			if (!next_expr) {
				throw MathISyntaxError(
					_Src,
					"the expression is expected",
					_Tokens.at(_Offset + 1).start,
					_Tokens.at(_Offset + 2).end
				);

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

bool expr_par_rule(std::vector<Token>& _Tokens, size_t _Offset, const std::string_view& _Src) {
	TokenType type0 = _Tokens.at(_Offset).type;

	if (type0 != sym_lpar)
		return false;

	if (!is_elem_exist(_Tokens, _Offset + 1)) {
		throw MathISyntaxError(
			_Src,
			"after '(' the expression is expected",
			_Tokens.at(_Offset).start,
			_Tokens.at(_Offset).end
		);

		return false;
	}

	bool next_expr;
	size_t current_size;

	do {
		current_size = _Tokens.size();

		next_expr = expr_rule(_Tokens, _Offset + 1, _Src);

		if (!next_expr && (current_size != _Tokens.size())) {
			throw MathISyntaxError(
				_Src,
				"after '(' the expression is expected",
				_Tokens.at(_Offset).start,
				_Tokens.at(_Offset + 1).end
			);

			return false;
		}

	} while (current_size != _Tokens.size());

	if (_Tokens.at(_Offset + 1).type == stmt_func_decl) {
		throw MathISyntaxError(
			_Src,
			"after '(' the expression is expected",
			_Tokens.at(_Offset).start,
			_Tokens.at(_Offset + 1).end
		);

		return false;
	}

	if (!is_elem_exist(_Tokens, _Offset + 2)) {
		throw MathISyntaxError(
			_Src,
			"after expression the ')' is expected",
			_Tokens.at(_Offset + 1).start,
			_Tokens.at(_Offset + 1).end
		);

		return false;
	}

	TokenType type2 = _Tokens.at(_Offset + 2).type;

	if (type2 != sym_rpar) {
		throw MathISyntaxError(
			_Src,
			"after expression the ')' is expected",
			_Tokens.at(_Offset + 1).start,
			_Tokens.at(_Offset + 2).end
		);

		return false;
	}

	_Tokens.erase(std::next(_Tokens.begin(), _Offset));
	_Tokens.erase(std::next(_Tokens.begin(), _Offset));
	_Tokens.at(_Offset).type = expr_par;

	return true;
}

bool expr_func_decl_rule(std::vector<Token>& _Tokens, size_t _Offset, const std::string_view& _Src) {
	if (_Tokens.at(_Offset).type != expr_var)
		return false;

	if (!is_elem_exist(_Tokens, _Offset + 1))
		return false;

	if (_Tokens.at(_Offset + 1).type != sym_lpar)
		return false;

	if (!is_elem_exist(_Tokens, _Offset + 2)) {
		throw MathISyntaxError(
			_Src,
			"after '(' the expected is expected",
			_Tokens.at(_Offset + 1).start,
			_Tokens.at(_Offset + 1).end
		);

		return false;
	}

	bool next_expr;
	size_t current_size;

	do {
		current_size = _Tokens.size();

		next_expr = expr_rule(_Tokens, _Offset + 2, _Src);

		if (!next_expr && (current_size != _Tokens.size())) {
			throw MathISyntaxError(
				_Src,
				"after '(' the expression is expected",
				_Tokens.at(_Offset + 1).start,
				_Tokens.at(_Offset + 2).end
			);

			return false;
		}

	} while (current_size != _Tokens.size());

	if (!is_elem_exist(_Tokens, _Offset + 3)) {
		throw MathISyntaxError(
			_Src,
			"after expression the ')' is expected",
			_Tokens.at(_Offset + 2).start,
			_Tokens.at(_Offset + 2).end
		);

		return false;
	}

	bool next_sym_rpar = _Tokens.at(_Offset + 3).type == sym_rpar;

	if (!next_sym_rpar) {
		throw MathISyntaxError(
			_Src,
			"after expression the ')' is expected",
			_Tokens.at(_Offset + 2).start,
			_Tokens.at(_Offset + 3).end
		);

		return false;
	}

	TokenType expr_type = _Tokens.at(_Offset + 2).type;

	bool next_assign_op = (_Tokens.size() > _Offset + 5) && (_Tokens.at(_Offset + 4).type == bin_op_assign);

	if ((expr_type == var_enum || expr_type == expr_var) && next_assign_op) {
		while (is_elem_exist(_Tokens, _Offset + 6)) {
			next_expr = expr_rule(_Tokens, _Offset + 5, _Src);
		}

		if (!next_expr || (_Tokens.at(_Offset + 5).type == stmt_func_decl)) {
			throw MathISyntaxError(
				_Src,
				"after '=' the expression is expected",
				_Tokens.at(_Offset + 4).start,
				_Tokens.at(_Offset + 5).end
			);

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

void rule(std::vector<Token>& _Tokens, const std::string_view& _Src) {
	do {
		bool res = expr_rule(_Tokens, 0, _Src);
		if (!res)
			return;

	} while (_Tokens.size() != 1);
}
