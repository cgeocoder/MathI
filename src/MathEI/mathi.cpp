#include "mathi.h"
#include "parser.h"
#include <map>
#include <cmath>

MathI::MathI() {

	// builtin_functions.push_back({ "abs", std::fabs<long double>, 1 });
}

std::vector<std::pair<size_t, size_t>> divide_into_expr(const std::string& _Str) {
	const size_t length = _Str.length();
	std::vector<std::pair<size_t, size_t>> res;

	for (size_t i = 0, j = std::min(_Str.find(';', i), length); i < length; i = j + 1, j = std::min(_Str.find(';', j + 1), length)) {
		res.push_back({ i, j });
	}

	return res;
}

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
		// __debugbreak();

		for (size_t i = range.first + 1; i < range.second; ++i) {
			if ((ast.at(i)->token.type == type) && (ast.at(i - 1)->token.type != sym_rpar) && (ast.at(i - 1)->token.type != expr_par))
				return i;
		}
	}
	else if (type == bin_op_pow) {
		// right-associative operator

		for (long long i = range.second - 1; i >= (long long)range.first; --i) {
			if (ast.at(i)->token.type == type && ast.at(i)->nodes.empty())
				return i;
		}
	}
	else {
		for (size_t i = range.first; i < range.second; ++i) {
			if (ast.at(i)->token.type == type && ast.at(i)->nodes.empty())
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
		&& (ast.at(3)->token.type >= bin_op_pow && ast.at(3)->token.type <= var_enum))
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

	while ((operator_pos = find_type_in_range_ast(bin_op_pow, ast, range)) != std::string::npos) {
		// ast.at(operator_pos)->token.type = expr_bin_op;
		ast.at(operator_pos)->nodes.push_back(ast.at(operator_pos - 1));
		ast.at(operator_pos)->nodes.push_back(ast.at(operator_pos + 1));

		ast.erase(ast.begin() + operator_pos + 1);
		ast.erase(ast.begin() + operator_pos - 1);

		range.second -= 2;
	}

	// 2. un_op_sub

	while ((operator_pos = find_type_in_range_ast(un_op_sub, ast, range)) != std::string::npos) {
		// ast.at(operator_pos)->token.type = expr_un_op;
		ast.at(operator_pos)->nodes.push_back(ast.at(operator_pos + 1));

		ast.erase(std::next(ast.begin(), operator_pos + 1));

		range.second -= 1;
	}

	// 3. bin_op_mul - bool_bin_op_or

	for (TokenType op = bin_op_mul; op <= bool_bin_op_or; op = (TokenType)(op + 1)) {
		while ((operator_pos = find_type_in_range_ast(op, ast, range)) != std::string::npos) {
			// ast.at(operator_pos)->token.type = expr_bin_op;
			ast.at(operator_pos)->nodes.push_back(ast.at(operator_pos - 1));
			ast.at(operator_pos)->nodes.push_back(ast.at(operator_pos + 1));

			ast.erase(ast.begin() + operator_pos + 1);
			ast.erase(ast.begin() + operator_pos - 1);

			range.second -= 2;
		}
	}

	// 4. un_op_not

	while ((operator_pos = find_type_in_range_ast(bool_un_op_not, ast, range)) != std::string::npos) {
		// ast.at(operator_pos)->token.type = expr_un_op;
		ast.at(operator_pos)->nodes.push_back(ast.at(operator_pos + 1));

		ast.erase(ast.begin() + operator_pos + 1);

		range.second -= 1;
	}

	// 5. enum

	while ((operator_pos = find_type_in_range_ast(sym_comma, ast, range)) != std::string::npos) {
		if (ast.at(operator_pos - 1)->token.type == TokenType::expr_enum) {
			ast.at(operator_pos - 1)->nodes.push_back(ast.at(operator_pos + 1));

			ast.erase(ast.begin() + operator_pos);
			ast.erase(ast.begin() + operator_pos);
		}
		else {
			ast.at(operator_pos)->token.type = expr_enum;
			ast.at(operator_pos)->nodes.push_back(ast.at(operator_pos - 1));
			ast.at(operator_pos)->nodes.push_back(ast.at(operator_pos + 1));

			ast.erase(ast.begin() + operator_pos + 1);
			ast.erase(ast.begin() + operator_pos - 1);
		}

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
		ast.at(range.first)->token.type = expr_par;
		ast.at(range.first)->token.value = "expr_par";
		ast.at(range.first)->nodes.push_back(ast.at(range.first + 1));

		// ast.at(range.first) = ast.at(range.first + 1);

		ast.erase(ast.begin() + range.first + 1);
		ast.erase(ast.begin() + range.first + 1);
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

int MathI::get_symbol_id(const std::string& _Name) {
	for (size_t i = 0; i < symbol_table.size(); ++i) {
		if (symbol_table.at(i).name == _Name)
			return (int)i;
	}

	symbol_table.push_back({ _Name });
	return (int)symbol_table.size() - 1;
}

void MathI::generate_ast(const std::vector<Token>& _Tokens) {
	std::vector<AST*> ast;

	for (auto& token : _Tokens) {
		ast.push_back(new AST(token));
	}

	while (ast.size() != 1) {
		std::pair<size_t, size_t> parse_range = get_parse_range(ast);

		generate_range_ast(ast, parse_range);
	}

	// print_ast(ast.at(0));
	// std::cout << '\n';

	m_AST = ast.at(0);
}

void MathI::r_gen_opcode(std::vector<Opcode>& opcode, AST* ast, const Function& func) {
	Token& tok = ast->token;

	if (tok.type == TokenType::expr_par) {
		ast = ast->nodes.at(0);
		tok = ast->token;
	}

	if (tok.type == TokenType::stmt_func_decl) {
		// check function arguments

		std::vector<std::string> args;
		std::vector<AST*> param_enum;

		if (ast->nodes[0]->nodes[0]->token.type == expr_enum) {
			param_enum = ast->nodes[0]->nodes[0]->nodes;
		}
		else {
			param_enum = ast->nodes[0]->nodes;
		}

		// decl -> par -> arg0, arg1, ..., argN
		for (auto& arg : param_enum) {
			for (auto& name : args) {
				if (name == arg->token.value) {
					printf("mathi: code gen error: function declaration has a similar parameter name\n");
					return;
				}
			}

			args.push_back(arg->token.value);
		}

		Function func_decl;
		func_decl.name = ast->token.value;
		func_decl.arg_name = args;
		func_decl.args = new double[func.arg_name.size()];
		func_decl.addr = functions.size();

		for (long long i = param_enum.size() - 1; i >= 0; --i) {
			func_decl.opcode.push_back(Opcode::store_tmp);
			func_decl.opcode.push_back((Opcode)func_decl.addr);
			func_decl.opcode.push_back((Opcode)i);
		}

		r_gen_opcode(func_decl.opcode, ast->nodes.at(1), func_decl);
		func_decl.opcode.push_back(Opcode::halt);

		functions.push_back(func_decl);

		// printf("# function %llu\n", func_decl.addr);
		// debug_print_opcode(func_decl.opcode);
	}
	else if (tok.type == expr_num_const) {
		opcode.push_back(Opcode::push_const);

		double val = std::stod(tok.value);
		size_t f_val;
		std::memcpy(&f_val, &val, sizeof(val));

		opcode.push_back((Opcode)f_val);
	}
	else if (tok.type == expr_var) {
		if (!func.arg_name.empty()) {
			std::string& name = tok.value;
			bool added = false;

			for (size_t i = 0; i < func.arg_name.size(); ++i) {
				if (name == func.arg_name.at(i)) {
					opcode.push_back(Opcode::push_tmp);
					opcode.push_back((Opcode)func.addr);
					opcode.push_back((Opcode)i);
					added = true;
					break;
				}
			}

			if (!added) {
				opcode.push_back(Opcode::push);
				opcode.push_back((Opcode)get_symbol_id(tok.value));
			}
		}
		else {
			opcode.push_back(Opcode::push);
			opcode.push_back((Opcode)get_symbol_id(tok.value));
		}
	}
	else if (tok.type == bin_op_assign) {
		r_gen_opcode(opcode, ast->nodes[1], func);

		opcode.push_back(Opcode::store);

		Token& dest_tok = ast->nodes[0]->token;
		opcode.push_back((Opcode)get_symbol_id(dest_tok.value));
	}
	else if (tok.type >= bin_op_pow && tok.type <= bool_bin_op_or) {
		r_gen_opcode(opcode, ast->nodes[0], func);
		r_gen_opcode(opcode, ast->nodes[1], func);

		opcode.push_back(Opcode::bin_op);
		opcode.push_back((Opcode)tok.type);
	}
	else if (tok.type >= bool_un_op_not && tok.type <= un_op_sub) {
		r_gen_opcode(opcode, ast->nodes[0], func);

		opcode.push_back(Opcode::un_op);
		opcode.push_back((Opcode)tok.type);
	}
	else if (tok.type == expr_func_call) {
		std::vector<AST*>& params = ast->nodes[0]->nodes; // [0] ->nodes;
		bool added = false;

		if (params.at(0)->token.type == expr_enum) {
			params = params[0]->nodes;
		}

		for (auto& node : params) {
			r_gen_opcode(opcode, node, func);
		}

		for (size_t i = 0; i < functions.size(); ++i) {
			Function& func = functions.at(i);

			if (tok.value == func.name) {
				opcode.push_back(Opcode::call);
				opcode.push_back((Opcode)i);
				opcode.push_back((Opcode)params.size());

				added = true;
			}
		}

		if (!added) {
			printf("mathi: code gen error: function '%s' is not defined\n", tok.value.c_str());
			return;
		}
	}

	// __debugbreak();
}

void MathI::debug_print_opcode(const std::vector<Opcode>& _Opcode) {
	for (size_t i = 0; i < _Opcode.size(); ++i) {

		if (_Opcode.at(i) == Opcode::push) {
			printf("%d:\tpush\t%d\t\t(%s)\n", (int)i, (int)_Opcode.at(i + 1),
				symbol_table.at((size_t)_Opcode.at(i + 1)).name.c_str());

			i += 1;
		}

		else if (_Opcode.at(i) == Opcode::push_const) {
			size_t f_val = (size_t)_Opcode.at(i + 1);
			double val;
			std::memcpy(&val, &f_val, sizeof(f_val));

			printf("%d:\tpush_c\t%f\n", (int)i, val);
			i += 1;
		}

		else if (_Opcode.at(i) == Opcode::push_tmp) {
			size_t func_addr = (size_t)_Opcode.at(i + 1);
			size_t index = (size_t)_Opcode.at(i + 2);

			printf("%d:\tpush_tmp\t%llu\t%llu\n", (int)i, func_addr, index);
			i += 2;
		}

		else if (_Opcode.at(i) == Opcode::store_tmp) {
			size_t func_addr = (size_t)_Opcode.at(i + 1);
			size_t index = (size_t)_Opcode.at(i + 2);

			printf("%d:\tstore_tmp\t%llu\t%llu\n", (int)i, func_addr, index);
			i += 2;
		}

		else if (_Opcode.at(i) == Opcode::pop) {
			printf("%d:\tpop\n", (int)i);
		}

		else if (_Opcode.at(i) == Opcode::bin_op) {
			switch (_Opcode.at(i + 1))
			{
			case Opcode::bo_pow:
				printf("%d:\tbin_op\tpow\n", (int)(i++));
				break;

			case Opcode::bo_mul:
				printf("%d:\tbin_op\tmul\n", (int)(i++));
				break;

			case Opcode::bo_div:
				printf("%d:\tbin_op\tdiv\n", (int)(i++));
				break;

			case Opcode::bo_add:
				printf("%d:\tbin_op\tadd\n", (int)(i++));
				break;

			case Opcode::bo_sub:
				printf("%d:\tbin_op\tsub\n", (int)(i++));
				break;

			case Opcode::bo_less:
				printf("%d:\tbin_op\tless\n", (int)(i++));
				break;

			case Opcode::bo_less_eq:
				printf("%d:\tbin_op\tless_eq\n", (int)(i++));
				break;

			case Opcode::bo_more:
				printf("%d:\tbin_op\tmore\n", (int)(i++));
				break;

			case Opcode::bo_more_eq:
				printf("%d:\tbin_op\tmore_eq\n", (int)(i++));
				break;

			case Opcode::bo_eq:
				printf("%d:\tbin_op\teq\n", (int)(i++));
				break;

			case Opcode::bo_not_eq:
				printf("%d:\tbin_op\tnot_eq\n", (int)(i++));
				break;

			case Opcode::bo_and:
				printf("%d:\tbin_op\tand\n", (int)(i++));
				break;

			case Opcode::bo_or:
				printf("%d:\tbin_op\tor\n", (int)(i++));
				break;

			default:
				__debugbreak();
				break;
			}
		}

		else if (_Opcode.at(i) == Opcode::un_op) {
			if (_Opcode.at(i + 1) == Opcode::uo_neg) {
				printf("%d:\tun_op\tneg\n", (int)(i++));
			}
			else if (opcode.at(i + 1) == Opcode::uo_not) {
				printf("%d:\tun_op\tnot\n", (int)(i++));
			}
			else {
				__debugbreak();
			}
		}

		else if (_Opcode.at(i) == Opcode::call) {
			printf("%d:\tcall\t%llu\t%llu\n", (int)i, _Opcode.at(i + 1), _Opcode.at(i + 2));
			i += 2;
		}

		else if (_Opcode.at(i) == Opcode::store) {
			printf("%d:\tstore\t%d\t\t(%s)\n", (int)i, (int)_Opcode.at(i + 1),
				symbol_table.at((size_t)_Opcode.at(i + 1)).name.c_str());

			i += 1;
		}

		else if (_Opcode.at(i) == Opcode::clear_stack) {
			printf("%d:\tclear_stack\n", (int)i);
		}

		else if (_Opcode.at(i) == Opcode::halt) {
			printf("%d:\thalt\n", (int)i);
			break;
		}
	}
}

void MathI::gen_executable() {
	opcode.clear();
	r_gen_opcode(opcode, m_AST);
	
	opcode.push_back(Opcode::halt);

	// debug_print_opcode(opcode);

	// __debugbreak();
}

double MathI::execute(std::vector<Opcode>& _Opcode) {
	Function* funcs = functions.data();
	Object* objects = symbol_table.data();
	Opcode* instructions = _Opcode.data();
	size_t i = 0;

	while (instructions[i] != Opcode::halt) {
		Opcode inst0 = instructions[i];
		Opcode inst1 = instructions[i + 1];

		switch (inst0)
		{
		case Opcode::push: {
			Object& obj = objects[(size_t)inst1];

			if (!obj.init) {
				printf("mathi: runtime error: variable '%s' is not initialized\n", obj.name.c_str());
				return -1.0;
			}

			if (max_stack_length == stack_counter) {
				printf("mathi: runtime error: stack overflow\n");
				return -1.0;
			}

			stack[stack_counter] = obj.value;
			stack_counter += 1;
			i += 1;
			break;
		}

		case Opcode::push_const: {
			if (max_stack_length == stack_counter) {
				printf("mathi: runtime error: stack overflow\n");
				return -1.0;
			}

			std::memcpy(&stack[stack_counter], &inst1, sizeof(Opcode));

			stack_counter += 1;
			i += 1;
			break;
		}
		case Opcode::push_tmp: {
			size_t func_addr = (size_t)inst1;
			size_t index = (size_t)instructions[i + 2];

			Function& func = funcs[func_addr];
			stack[stack_counter] = func.args[index];

			stack_counter += 1;
			i += 2;

			break;
		}

		case Opcode::store_tmp: {
			size_t func_addr = (size_t)inst1;
			size_t index = (size_t)instructions[i + 2];

			Function& func = funcs[func_addr];
			func.args[index] = stack[--stack_counter];

			i += 2;

			break;
		}

		case Opcode::pop:
			stack_counter -= 1;
			break;
		case Opcode::bin_op: {
			double left_val = stack[stack_counter - 2];
			double right_val = stack[stack_counter - 1];
			stack_counter -= 1;
			double res = 0.0;

			switch (inst1) {
			case Opcode::bo_pow:		res = std::pow(left_val, right_val); break;
			case Opcode::bo_mul:		res = left_val * right_val; break;
			case Opcode::bo_div:		res = left_val / right_val; break;
			case Opcode::bo_add:		res = left_val + right_val; break;
			case Opcode::bo_sub:		res = left_val - right_val; break;
			case Opcode::bo_less:		res = (double)(left_val < right_val); break;
			case Opcode::bo_less_eq:	res = (double)(left_val <= right_val); break;
			case Opcode::bo_more:		res = (double)(left_val > right_val); break;
			case Opcode::bo_more_eq:	res = (double)(left_val >= right_val); break;
			case Opcode::bo_eq:			res = (double)(left_val == right_val); break;
			case Opcode::bo_not_eq:		res = (double)(left_val != right_val); break;
			case Opcode::bo_and:		res = (double)((bool)left_val && (bool)right_val); break;
			case Opcode::bo_or:			res = (double)((bool)left_val || (bool)right_val); break;
			}

			stack[stack_counter - 1] = res;
			i += 1;
			break;
		}
		case Opcode::un_op: {
			double val = stack[stack_counter - 1];
			double res = 0.0;

			switch (inst1) {
			case Opcode::uo_neg:
				res = -val;
				break;
			case Opcode::uo_not:
				res = (double)(!((bool)val));
				break;
			}

			stack[stack_counter - 1] = res;

			i += 1;

			break;
		}
		case Opcode::call: {
			size_t func_addr = (size_t)inst1;
			size_t p_count = (size_t)instructions[i + 2];

			Function& func = funcs[func_addr];

			if (func.arg_name.size() != p_count) {
				printf("mathi: runtime error: function '%s' has a %llu arguments, you passed %llu\n", 
					func.name.c_str(), func.arg_name.size(), p_count);
				return -1.0;
			}

			size_t stack_frame = stack_counter;

			stack[stack_frame] = execute(func.opcode);
			stack_counter = stack_frame + 1;
			i += 2;

			break;
		}
		case Opcode::store: {
			size_t index = (size_t)inst1;

			double val = stack[stack_counter - 1];
			stack_counter -= 1;

			objects[index].init = true;
			objects[index].value = val;

			i += 1;

			break;
		}
		case Opcode::clear_stack:
			stack_counter = 0;
			break;
		case Opcode::halt:
			break;
		default:
			break;
		}

		i += 1;
	}

	if (stack_counter == 0)
		return 0.0;

	return stack[stack_counter - 1];
}

double MathI::eval(const std::string& _Str) {
	global_rule_state = true;

	double result = 0.0;

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

		generate_ast(tokens.m_Tokens);

		stack_counter = 0;
		gen_executable();

		result = execute(opcode);
	}

	return result;
};