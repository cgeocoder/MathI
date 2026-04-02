#include "mathi.h"
#include "parser.h"
#include <functional>
#include <stack>

bool global_rule_state = true;

static bool expr_bin_op_rule(std::vector<Token>&, size_t);
static bool expr_enum_rule(std::vector<Token>&, size_t);
static bool expr_un_op_rule(std::vector<Token>&, size_t);
static bool expr_par_rule(std::vector<Token>&, size_t);
static bool expr_var_rule(std::vector<Token>&, size_t);
static bool expr_num_const_rule(std::vector<Token>&, size_t);
static bool expr_func_decl_rule(std::vector<Token>&, size_t);
// static bool expr_cond_rule(std::vector<Token>&, size_t);

static bool expr_rule(std::vector<Token>& _Tokens, size_t _Offset) {
	if (global_rule_state && expr_un_op_rule(_Tokens, _Offset))
		return true;

	if (global_rule_state && expr_bin_op_rule(_Tokens, _Offset))
		return true;

	if (global_rule_state && expr_func_decl_rule(_Tokens, _Offset))
		return true;

	if (global_rule_state && expr_par_rule(_Tokens, _Offset))
		return true;

	if (global_rule_state && expr_enum_rule(_Tokens, _Offset))
		return true;

	if (global_rule_state && (expr_var_rule(_Tokens, _Offset) || expr_num_const_rule(_Tokens, _Offset)))
		return true;

	return false;
}

static bool expr_var_rule(std::vector<Token>& _Tokens, size_t _Offset) {
	bool current_var = _Tokens.at(_Offset).type == expr_var;

	if (!current_var)
		return false;

	if (_Tokens.size() > _Offset + 1) {
		TokenType next_type = _Tokens.at(_Offset + 1).type;

		bool next_expr = (next_type >= expr_num_const && next_type <= var_enum);

		if (next_expr) {
			std::cout << "The expr is not expected after expr: ["
				<< _Tokens.at(_Offset + 1).start << ", "
				<< _Tokens.at(_Offset + 1).end << "]\n";
			return global_rule_state = false;
		}
	}

	return true;
}

static bool expr_un_op_rule(std::vector<Token>& _Tokens, size_t _Offset) {
	if (_Tokens.size() <= _Offset + 1)
		return false;

	TokenType current_type = _Tokens.at(_Offset).type;

	bool current_sign = current_type == bin_op_sub || current_type == bool_un_op_not;

	if (!current_sign)
		return false;

	bool next_expr = expr_rule(_Tokens, _Offset + 1);

	if (!next_expr) {
		std::cout << "The expr is expected after unary operator '-': ["
			<< _Tokens.at(_Offset + 1).start << ", "
			<< _Tokens.at(_Offset + 1).end << "]\n";

		return global_rule_state = false;
	}

	_Tokens.erase(std::next(_Tokens.begin(), _Offset));
	_Tokens.at(_Offset).type = expr_un_op;

	return true;
}

static bool expr_num_const_rule(std::vector<Token>& _Tokens, size_t _Offset) {
	bool current_num = _Tokens.at(_Offset).type == expr_num_const;

	if (!current_num)
		return false;

	if (_Tokens.size() > _Offset + 1) {
		TokenType next_type = _Tokens.at(_Offset + 1).type;

		bool next_expr = (next_type >= expr_num_const && next_type <= var_enum);

		if (next_expr) {
			std::cout << "The expr is not expected after num_const: ["
				<< _Tokens.at(_Offset + 1).start << ", "
				<< _Tokens.at(_Offset + 1).end << "]\n";

			return global_rule_state = false;
		}
	}

	return true;
}

static bool expr_bin_op_rule(std::vector<Token>& _Tokens, size_t _Offset) {
	if (_Tokens.size() <= _Offset + 2)
		return false;

	TokenType type0 = _Tokens.at(_Offset).type;
	
	if (type0 >= expr_num_const && type0 <= var_enum) {
		TokenType type1 = _Tokens.at(_Offset + 1).type;

		if (type1 >= bin_op_pow && type1 <= bool_bin_op_or) {
			if (type1 == bin_op_assign && type0 != expr_var) {
				std::cout << "the '=' operator can only go after variable name: ["
					<< _Tokens.at(_Offset).start << ", "
					<< _Tokens.at(_Offset + 1).end << "]\n";

				return global_rule_state = false;
			}

			bool next_expr = expr_rule(_Tokens, _Offset + 2);

			if (_Tokens.at(_Offset + 2).type == stmt_func_decl) {
				std::cout << "After the name '" << _Tokens.at(_Offset + 2).value << "' the expression is expected: ["
					<< _Tokens.at(_Offset + 1).start << ", "
					<< _Tokens.at(_Offset + 2).end << "]\n";

				return global_rule_state = false;
			}

			if (!next_expr) {
				std::cout << "The expr is expected: ["
					<< _Tokens.at(_Offset + 2).start << ", "
					<< _Tokens.at(_Offset + 2).end << "]\n";

				return global_rule_state = false;
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
	if (_Tokens.size() <= _Offset + 2)
		return false;

	TokenType type0 = _Tokens.at(_Offset).type;
	
	if (type0 >= expr_num_const && type0 <= var_enum) {
		TokenType type1 = _Tokens.at(_Offset + 1).type;

		if (type1 == sym_comma) {
			bool next_expr = expr_rule(_Tokens, _Offset + 2);

			if (!next_expr) {
				std::cout << "The expr is expected: ["
					<< _Tokens.at(_Offset + 2).start << ", "
					<< _Tokens.at(_Offset + 2).end << "]\n";

				return global_rule_state = false;
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
	if (_Tokens.size() <= _Offset + 2)
		return false;

	TokenType type0 = _Tokens.at(_Offset).type;

	if (type0 == sym_lpar) {
		bool next_expr = expr_rule(_Tokens, _Offset + 1);

		if (!next_expr) {
			std::cout << "after '(' the expr expected: ["
				<< _Tokens.at(_Offset + 1).start << ", "
				<< _Tokens.at(_Offset + 1).end << "]\n";

			return global_rule_state = false;
		}

		if (_Tokens.size() <= _Offset + 2)
			return false;

		TokenType type2 = _Tokens.at(_Offset + 2).type;

		if (type2 != sym_rpar) {
			std::cout << "after expr the ')' expected: ["
				<< _Tokens.at(_Offset + 2).start << ", "
				<< _Tokens.at(_Offset + 2).end << "]\n";

			return global_rule_state = false;
		}

		_Tokens.erase(std::next(_Tokens.begin(), _Offset));
		_Tokens.erase(std::next(_Tokens.begin(), _Offset));
		_Tokens.at(_Offset).type = expr_par;

		return true;
	}

	return false;
}

static bool expr_func_decl_rule(std::vector<Token>& _Tokens, size_t _Offset) {
	if (_Tokens.size() <= _Offset + 3)
		return false;

	if (_Tokens.at(_Offset).type != expr_var)
		return false;

	if (_Tokens.at(_Offset + 1).type != sym_lpar)
		return false;

	bool next_expr = expr_rule(_Tokens, _Offset + 2);

	if (!next_expr) {
		std::cout << "The expr is expected after '(': ["
			<< _Tokens.at(_Offset + 2).start << ", "
			<< _Tokens.at(_Offset + 2).end << "]\n";

		return global_rule_state = false;
	}

	bool next_sym_rpar = (_Tokens.size() > _Offset + 3) && (_Tokens.at(_Offset + 3).type == sym_rpar);

	if (!next_sym_rpar) {
		std::cout << "The ')' is expected after expr: ["
			<< _Tokens.at(_Offset + 3).start << ", "
			<< _Tokens.at(_Offset + 3).end << "]\n";

		return global_rule_state = false;
	}

	// Is func decl
	TokenType expr_type = _Tokens.at(_Offset + 2).type;

	bool next_assign_op = (_Tokens.size() > _Offset + 5) && (_Tokens.at(_Offset + 4).type == bin_op_assign);

	if ((expr_type == var_enum || expr_type == expr_var) && next_assign_op) {
		next_expr = expr_rule(_Tokens, _Offset + 5);

		if (!next_expr) {
			std::cout << "The expr is expected after '=': ["
				<< _Tokens.at(_Offset + 5).start << ", "
				<< _Tokens.at(_Offset + 5).end << "]\n";

			return global_rule_state = false;
		}

		_Tokens.erase(std::next(_Tokens.begin(), _Offset));
		_Tokens.erase(std::next(_Tokens.begin(), _Offset));
		_Tokens.erase(std::next(_Tokens.begin(), _Offset));
		_Tokens.erase(std::next(_Tokens.begin(), _Offset));
		_Tokens.erase(std::next(_Tokens.begin(), _Offset));
		_Tokens.at(_Offset).type = stmt_func_decl;
	}
	else {		_Tokens.erase(std::next(_Tokens.begin(), _Offset));
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

		return global_rule_state = false;
	}

	next_expr = (_Tokens.size() > _Offset + 4) && expr_rule(_Tokens, _Offset + 4);

	if (!next_expr) {
		std::cout << "The expr is expected after ',': ["
			<< _Tokens.at(_Offset + 4).start << ", "
			<< _Tokens.at(_Offset + 4).end << "]\n";

		return global_rule_state = false;
	}

	if (_Tokens.size() <= _Offset + 5) {
		std::cout << "The 'if'/'else' is expected after expr: ["
			<< _Tokens.at(_Offset + 4).start << ", "
			<< _Tokens.at(_Offset + 4).end << "]\n";

		return global_rule_state = false;
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

		return global_rule_state = false;
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
	const size_t length = _Str.length();

	for (size_t i = 0, j = std::min(_Str.find(';', i), length); i < length; i = j + 1, j = std::min(_Str.find(';', j + 1), length)) {
		Tokenizer tokens;

		if (!tokens.parse(_Str.substr(i, j - i))) {
			break;
		}

		std::vector<Token> token_array;
		for (auto& tok : tokens.m_Tokens)
			token_array.push_back(tok);

		bool current_expr = rule(token_array);

		if (!current_expr)
			std::cout << "This is not an Expression\n\n";
	}

	return 0.0;
};