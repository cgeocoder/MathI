#include "tokens.h"

bool is_expr(TokenType* t) {
	return (*t >= expr_num_const)
		&& (*t <= expr);
}

bool cond_after_expr(TokenType* t) {
	return t[0] != sym_lpar;
}

bool is_bin_op(TokenType* t) {
	return (*t >= bin_op_pow)
		&& (*t <= bin_op_assign);
}

// Look ahead - 4
bool is_expr_bin_op(TokenType* t) {
	return is_expr(&t[0])
		&& is_bin_op(&t[1])
		&& is_expr(&t[2])
		&& cond_after_expr(&t[3]);
}

// Look ahead - 2
bool is_expr_par(TokenType* t) {
	return t[0] == sym_lpar
		&& is_expr(&t[1])
		&& t[2] == sym_rpar;
}

// Look ahead - 1
bool is_expr_func_call(TokenType* t) {
	return t[0] == expr_var
		&& t[1] == expr_par;
}

// Look ahead - 3
bool is_expr_enum(TokenType* t) {
	return is_expr(&t[0])
		&& t[1] == sym_comma
		&& (is_expr(&t[2]) || t[2] == expr_enum)
		&& cond_after_expr(&t[3]);
}

bool is_un_op(TokenType* t) {
	return *t == bin_op_sub;
}

// Look ahead - 2
bool is_expr_un_op(TokenType* t) {
	return is_un_op(&t[0])
		&& is_expr(&t[1])
		&& cond_after_expr(&t[2]);
}

bool cond_before_expr_un_op(TokenType* t) {
	return t[0] != expr_num_const
		&& t[0] != expr_var
		&& t[0] != sym_rpar;
}

bool is_bool_bin_op(TokenType* t) {
	return (*t >= bool_bin_op_less)
		&& (*t <= bool_bin_op_or);
}

// Look ahead - 3
bool is_bool_expr_bin_op(TokenType* t) {
	return is_expr(&t[0])
		&& is_bool_bin_op(&t[1])
		&& is_expr(&t[2])
		&& cond_after_expr(&t[3]);
}

// Look ahead - 5
bool is_bool_expr_ter_op(TokenType* t) {
	return is_expr(&t[0])
		&& is_bool_bin_op(&t[1])
		&& is_expr(&t[2])
		&& is_bool_bin_op(&t[3])
		&& is_expr(&t[4])
		&& cond_after_expr(&t[5]);
}

// Look ahead - 2
bool is_bool_expr_un_op(TokenType* t) {
	return t[0] == bool_un_op_not
		&& is_expr(&t[1])
		&& cond_after_expr(&t[2]);
}

// Look ahead - 2
bool is_cond_if_part_enum(TokenType* t) {
	return t[0] == cond_if_part
		&& t[1] == sym_comma
		&& (t[2] == cond_if_part || t[2] == cond_if_part_enum);
}

// Look ahead - 1
bool is_cond_else_part(TokenType* t) {
	return is_expr(&t[0])
		&& t[1] == sym_else;
}

// Look ahead - 2
bool is_expr_cond(TokenType* t) {
	return (t[0] == cond_if_part || t[0] == cond_if_part_enum)
		&& t[1] == sym_comma
		&& t[2] == cond_else_part;
}

// Look ahead - 3
bool is_var_enum(TokenType* t) {
	return t[0] == expr_var
		&& t[1] == sym_comma
		&& (t[2] == expr_var || t[2] == var_enum)
		&& cond_after_expr(&t[3]);
}

// Look ahead - 6
bool is_stmt_func_decl(TokenType* t) {
	return t[0] == expr_var
		&& t[1] == sym_lpar
		&& (t[2] == expr_var || t[2] == var_enum)
		&& t[3] == sym_rpar
		&& t[4] == bin_op_assign
		&& is_expr(&t[5])
		&& cond_after_expr(&t[6]);
}

bool is_stmt(TokenType* t) {
	return t[0] == stmt_func_decl;
}

// Look ahead - 1
bool is_final_instruction(TokenType* t) {
	return (is_expr(&t[0]) || is_stmt(&t[0]))
		&& t[1] == sym_semicolon;
}

static bool is_name(char ch) {
	return (ch >= 'a') && (ch <= 'z')
		|| (ch >= 'A') && (ch <= 'Z')
		|| (ch == '_');
}

static bool is_number(char ch) {
	return (ch >= '0') && (ch <= '9');
}

static bool is_spec_symbol(char ch) {
	return ch == ','
		|| ch == ';'
		|| ch == '('
		|| ch == ')'
		|| ch == '^'
		|| ch == '*'
		|| ch == '/'
		|| ch == '+'
		|| ch == '-'
		|| ch == '='
		|| ch == '>'
		|| ch == '<';
}

static bool is_long_spec_symbol(char ch1, char ch2) {
	return (ch1 == '<' && ch2 == '=')
		|| (ch1 == '>' && ch2 == '=')
		|| (ch1 == '=' && ch2 == '=')
		|| (ch1 == '!' && ch2 == '=');
}

Tokenizer::Tokenizer() {}

bool Tokenizer::parse(const std::string& _Str) {
	size_t length = _Str.length();

	for (size_t i = 0; i < length; ++i) {
		if (_Str.at(i) == '#') {
			__debugbreak();
			return true;
		}
		else if (is_name(_Str.at(i))) {
			size_t end = i;

			while ((end < length) && (is_name(_Str.at(end)) || (_Str.at(end) >= '0' && _Str.at(end) <= '9') || _Str.at(end) == '_'))
				end += 1;

			std::string val = _Str.substr(i, end - i);

			if (val == "and") {
				m_Tokens.push_back({
					TokenType::bool_bin_op_and,
					_Str.substr(i, 3), i, i + 3
				});
			}
			else if (val == "or") {
				m_Tokens.push_back({
					TokenType::bool_bin_op_or,
					_Str.substr(i, 2), i, i + 2
				});
			}
			else if (val == "not") {
				m_Tokens.push_back({
					TokenType::bool_un_op_not,
					_Str.substr(i, 3), i, i + 3
				});
			}
			else if (val == "if") {
				m_Tokens.push_back({
					TokenType::sym_if,
					_Str.substr(i, 2), i, i + 2
				});
			}
			else if (val == "else") {
				m_Tokens.push_back({
					TokenType::sym_else,
					_Str.substr(i, 3), i, i + 3
				});
			}
			else {
				m_Tokens.push_back({
					TokenType::expr_var,
					_Str.substr(i, end - i), i, end
				});
			}

			i = end - 1;
		}
		else if (is_number(_Str.at(i))) {
			size_t end = i;
			bool dot = false;

			while (end < length && (is_number(_Str.at(end)) || (_Str.at(end) == '.'))) {
				if (dot && (_Str.at(end) == '.')) {
					std::cout << "Invalid floating constant '" << _Str.substr(i, end) << "'\n";
					return false;
				}
				else if (_Str.at(end) == '.')
					dot = true;

				end += 1;
			}

			m_Tokens.push_back({
				TokenType::expr_num_const,
				_Str.substr(i, end - i), i, end
			});

			i = end - 1;
		}
		else if (_Str.at(i) == ',') {
			m_Tokens.push_back({
				TokenType::sym_comma,
				_Str.substr(i, 1), i, i + 1
			});
		}
		else if (_Str.at(i) == ';') {
			m_Tokens.push_back({
				TokenType::sym_semicolon,
				_Str.substr(i, 1), i, i + 1
			});
		}
		else if (_Str.at(i) == '(') {
			m_Tokens.push_back({
				TokenType::sym_lpar,
				_Str.substr(i, 1), i, i + 1
			});
		}
		else if (_Str.at(i) == ')') {
			m_Tokens.push_back({
				TokenType::sym_rpar,
				_Str.substr(i, 1), i, i + 1
			});
		}
		else if (_Str.at(i) == '^') {
			m_Tokens.push_back({
				TokenType::bin_op_pow,
				_Str.substr(i, 1), i, i + 1
			});
		}
		else if (_Str.at(i) == '*') {
			m_Tokens.push_back({
				TokenType::bin_op_mul,
				_Str.substr(i, 1), i, i + 1
			});
		}
		else if (_Str.at(i) == '/') {
			m_Tokens.push_back({
				TokenType::bin_op_div,
				_Str.substr(i, 1), i, i + 1
			});
		}
		else if (_Str.at(i) == '+') {
			m_Tokens.push_back({
				TokenType::bin_op_add,
				_Str.substr(i, 1), i, i + 1
			});
		}
		else if (_Str.at(i) == '-') {
			m_Tokens.push_back({
				TokenType::bin_op_sub,
				_Str.substr(i, 1), i, i + 1
			});
		}
		else if (_Str.at(i) == '=') {
			if (i + 1 < length && _Str.at(i + 1) == '=') {
				m_Tokens.push_back({
					TokenType::bool_bin_op_eq,
					_Str.substr(i, 2), i, i + 2
				});

				i += 1;
			}
			else {
				m_Tokens.push_back({
					TokenType::bin_op_assign,
					_Str.substr(i, 1), i, i + 1
				});
			}
		}
		else if (_Str.at(i) == '!') {
			if (i + 1 < length && _Str.at(i + 1) == '=') {
				m_Tokens.push_back({
					TokenType::bool_bin_op_not_eq,
					_Str.substr(i, 2), i, i + 2
					});
				i += 1;
			}
			else {
				std::cout << "Invalid character constant '!'\n";
				return false;
			}
		}
		else if (_Str.at(i) == '<') {
			if (i + 1 < length && _Str.at(i + 1) == '=') {
				m_Tokens.push_back({
					TokenType::bool_bin_op_less_eq,
					_Str.substr(i, 2), i, i + 2
					});
				i += 1;
			}
			else {
				m_Tokens.push_back({
					TokenType::bool_bin_op_less,
					_Str.substr(i, 1), i, i + 1
					});
			}
		}
		else if (_Str.at(i) == '>') {
			if (i + 1 < length && _Str.at(i + 1) == '=') {
				m_Tokens.push_back({
					TokenType::bool_bin_op_more_eq,
					_Str.substr(i, 2), i, i + 2
					});
				i += 1;
			}
			else {
				m_Tokens.push_back({
					TokenType::bool_bin_op_more,
					_Str.substr(i, 1), i, i + 1
				});
			}
		}
	}

	return true;
}