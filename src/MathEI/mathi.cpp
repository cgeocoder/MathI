#include "mathi.h"
#include "parser.h"


std::vector<std::pair<size_t, size_t>> divide_into_expr(const std::string& _Str) {
	const size_t length = _Str.length();
	std::vector<std::pair<size_t, size_t>> res;

	for (size_t i = 0, j = std::min(_Str.find(';', i), length); i < length; i = j + 1, j = std::min(_Str.find(';', j + 1), length)) {
		res.push_back({ i, j });
	}

	return res;
}

class AST {
public:
	inline AST(const Token& _Token) : token{ _Token } {}

	Token token;
	std::vector<AST*> nodes;
};

std::pair<size_t, size_t> get_parse_range(const std::vector<AST*>& ast) {
	size_t lpar_depth = 0, 
		max_lpar_depth = 0, 
		max_lpar_depth_pos = 0;

	for (size_t i = 0; i < ast.size(); ++i) {
		TokenType type = ast.at(i)->token.type;

		if (type == sym_lpar) {
			lpar_depth += 1;

			if (lpar_depth > max_lpar_depth) {
				max_lpar_depth = lpar_depth;
				max_lpar_depth_pos = i;
			}
		}
		else if (type == sym_rpar) {
			lpar_depth -= 1;
		}
	}

	for (size_t i = max_lpar_depth_pos; i < ast.size(); ++i) {
		TokenType type = ast.at(i)->token.type;

		if (type == sym_rpar)
			return { max_lpar_depth_pos, i };
	}

	return { 0, ast.size() };
}

size_t find_type_in_range_ast(TokenType type, const std::vector<AST*>& ast, const std::pair<size_t, size_t>& range) {
	if (type == bin_op_assign) {
		for (size_t i = range.first + 1; i < range.second; ++i) {
			if ((ast.at(i)->token.type == type) && (ast.at(i - 1)->token.type != sym_rpar) && (ast.at(i - 1)->token.type != expr_par))
				return i;
		}
	}
	else if (type == bin_op_pow) {
		// right- associative operator

		for (long long i = range.second - 1; i >= (long long)range.first; --i) {
			if (ast.at(i)->token.type == type)
				return i;
		}
	}
	else {
		for (size_t i = range.first; i < range.second; ++i) {
			if (ast.at(i)->token.type == type)
				return i;
		}
	}

	return std::string::npos;
}

size_t find_func_call_in_range_ast(const std::vector<AST*>& ast, const std::pair<size_t, size_t>& range) {
	const size_t length = ast.size() - 1;

	for (size_t i = 0; i < length; ++i) {
		if ((ast.at(i)->token.type == expr_var) && (ast.at(i + 1)->token.type == expr_par)) {
			if ((i + 2 < length) && (ast.at(i + 2)->token.type != bin_op_assign))
				return i;

			else if ((i + 2 >= length))
				return i;
		}
	}

	return std::string::npos;
}

size_t find_func_decl_in_range_ast(const std::vector<AST*>& ast, const std::pair<size_t, size_t>& range) {
	if (ast.size() != 4)
		return std::string::npos;

	if ((ast.at(0)->token.type == expr_var) 
		&& (ast.at(1)->token.type == expr_par)
		&& (ast.at(2)->token.type == bin_op_assign)
		&& (ast.at(3)->token.type >= expr_num_const && ast.at(3)->token.type <= expr_enum))
			return 0;

	return std::string::npos;
}

void generate_range_ast(std::vector<AST*>& ast, std::pair<size_t, size_t> range) {
	size_t operator_pos;

	// 0. call

	while ((operator_pos = find_func_call_in_range_ast(ast, range)) != std::string::npos) {
		ast.at(operator_pos)->token.type = expr_func_call;
		ast.at(operator_pos)->nodes.push_back(ast.at(operator_pos + 1));

		ast.erase(ast.begin() + operator_pos + 1);

		range.second -= 1;
	}

	// 1. pow

	__debugbreak();

	while ((operator_pos = find_type_in_range_ast(bin_op_pow, ast, range)) != std::string::npos) {
		ast.at(operator_pos)->token.type = expr_bin_op;
		ast.at(operator_pos)->nodes.push_back(ast.at(operator_pos - 1));
		ast.at(operator_pos)->nodes.push_back(ast.at(operator_pos + 1));

		ast.erase(ast.begin() + operator_pos + 1);
		ast.erase(ast.begin() + operator_pos - 1);

		range.second -= 2;
	}

	// 2. un_op_sub

	while ((operator_pos = find_type_in_range_ast(un_op_sub, ast, range)) != std::string::npos) {
		ast.at(operator_pos)->token.type = expr_un_op;
		ast.at(operator_pos)->nodes.push_back(ast.at(operator_pos + 1));

		ast.erase(std::next(ast.begin(), operator_pos + 1));

		range.second -= 1;
	}

	// 3. bin_op_mul - bool_bin_op_or

	for (TokenType op = bin_op_mul; op <= bool_bin_op_or; op = (TokenType)(op + 1)) {
		while ((operator_pos = find_type_in_range_ast(op, ast, range)) != std::string::npos) {
			ast.at(operator_pos)->token.type = expr_bin_op;
			ast.at(operator_pos)->nodes.push_back(ast.at(operator_pos - 1));
			ast.at(operator_pos)->nodes.push_back(ast.at(operator_pos + 1));

			ast.erase(ast.begin() + operator_pos + 1);
			ast.erase(ast.begin() + operator_pos - 1);

			range.second -= 2;
		}
	}

	// 4. un_op_not

	while ((operator_pos = find_type_in_range_ast(bool_un_op_not, ast, range)) != std::string::npos) {
		ast.at(operator_pos)->token.type = expr_un_op;
		ast.at(operator_pos)->nodes.push_back(ast.at(operator_pos + 1));

		ast.erase(ast.begin() + operator_pos + 1);

		range.second -= 1;
	}

	// 5. enum

	while ((operator_pos = find_type_in_range_ast(sym_comma, ast, range)) != std::string::npos) {
		ast.at(operator_pos)->token.type = expr_enum;
		ast.at(operator_pos)->nodes.push_back(ast.at(operator_pos - 1));
		ast.at(operator_pos)->nodes.push_back(ast.at(operator_pos + 1));

		ast.erase(ast.begin() + operator_pos + 1);
		ast.erase(ast.begin() + operator_pos - 1);

		range.second -= 2;
	}

	// 6. func decl

	if ((operator_pos = find_func_decl_in_range_ast(ast, range)) != std::string::npos) {
		ast.at(operator_pos)->token.type = stmt_func_decl;
		ast.at(operator_pos)->nodes.push_back(ast.at(operator_pos + 1));
		ast.at(operator_pos)->nodes.push_back(ast.at(operator_pos + 3));

		ast.erase(ast.begin() + operator_pos + 1);
		ast.erase(ast.begin() + operator_pos + 1);
		ast.erase(ast.begin() + operator_pos + 1);

		range.second -= 3;
	}

	// 7. this paranthesis

	if (ast.at(range.first)->token.type == sym_lpar) {
		// __debugbreak();

		ast.at(range.first)->token.type = expr_par;
		ast.at(range.first)->token.value = "expr_par";
		ast.at(range.first)->nodes.push_back(ast.at(range.first + 1));

		ast.erase(ast.begin() + range.first + 1);
		ast.erase(ast.begin() + range.first + 1);

		// __debugbreak();
	}
}

void print_ast(AST* ast, unsigned int r = 0) {
	std::cout
		<< std::string(r, '|')
		<< "- "
		<< "'";
	
	if (ast->token.type == expr_func_call)
		std::cout << "call: " << ast->token.value;
	else if (ast->token.type == stmt_func_decl)
		std::cout << "decl: " << ast->token.value;
	else
		std::cout << ast->token.value;
	
	std::cout << "'" << '\n';
	
	for (auto& node : ast->nodes) {
		print_ast(node, r + 1);
	}
}

AST* geterate_ast(const std::vector<Token>& _Tokens) {
	std::vector<AST*> ast;

	for (auto& token : _Tokens) {
		ast.push_back(new AST(token));
	}

	__debugbreak();

	while (ast.size() != 1) {
		std::pair<size_t, size_t> parse_range = get_parse_range(ast);

		__debugbreak();

		generate_range_ast(ast, parse_range);
	}

	print_ast(ast.at(0));
	std::cout << '\n';

	return ast.at(0);
}

double MathI::eval(const std::string& _Str) {
	global_rule_state = true;

	for (auto& range : divide_into_expr(_Str)) {
		Tokenizer tokens;

		std::string sub_str = _Str.substr(range.first, range.second - range.first);
		if (!tokens.parse(sub_str)) {
			break;
		}

		std::vector<Token> token_array{ tokens.m_Tokens };

		is_condition_stack.push(false);
		bool current_expr = rule(token_array);
		is_condition_stack.pop();

		if (!current_expr) {
			print_syntax_error(sub_str, syntax_error_info);
			break;
		}

		AST* ast = geterate_ast(tokens.m_Tokens);

		// __debugbreak();
	}

	return 0.0;
};