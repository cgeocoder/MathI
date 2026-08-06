#include "mathi.h"
#include "parser.h"
#include "errors.h"
#include <cmath>
#include <algorithm>

typedef double (*b1func)(double);
typedef double (*b2func)(double, double);

std::vector<std::pair<size_t, size_t>> divide_into_expr(const std::string& _Str) {
	const size_t length = _Str.length();
	std::vector<std::pair<size_t, size_t>> res;

	for (size_t i = 0, j = std::min(_Str.find(';', i), length); i < length; i = j + 1, j = std::min(_Str.find(';', j + 1), length)) {
		res.push_back({ i, j });
	}

	return res;
}

// @return range [start, end)
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
			return { max_lpar_depth_pos, i + 1 }; // was just 'i'
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
	else if (type == bin_op_pow || type == un_op_sub || type == bool_un_op_not) {
		// right-associative operator

		auto rbegin_it = std::next(ast.rbegin(), ast.size() - range.second),
			rend_it = std::prev(ast.rend(), range.first);

		auto it = std::find_if(
			rbegin_it, rend_it,
			[&type](AST* node) -> bool {
				return node->token.type == type && node->nodes.empty();
			}
		);

		if (it != rend_it)
			return std::distance(ast.begin(), it.base() - 1);
	}
	else {
		auto begin_it = std::next(ast.begin(), range.first),
			end_it = std::prev(ast.end(), ast.size() - range.second);

		auto it = std::find_if(
			begin_it, end_it,
			[&type](AST* node) -> bool {
				return node->token.type == type && node->nodes.empty();
			}
		);

		if (it != end_it)
			return std::distance(ast.begin(), it);
	}

	return std::string::npos;
}

size_t find_func_call_in_range_ast(const std::vector<AST*>& ast, const std::pair<size_t, size_t>& range) {
	const size_t length = ast.size() - 1;

	for (size_t i = range.first; i < range.second - 1; ++i) {
		if ((ast.at(i)->token.type == expr_var) && (ast.at(i + 1)->token.type == expr_par)) {
			if (!is_elem_exist(ast, i + 2))
				return i;

			else if (ast.at(i + 2)->token.type != bin_op_assign)
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

size_t MathI::get_object_index(const MathIObject& _Obj) {
	auto it = std::find_if(m_Objects.begin(), m_Objects.end(), [&_Obj](const MathIObject& obj) {
		if (_Obj.constant && obj.constant) {
			MathIConstant* dest = static_cast<MathIConstant*>(_Obj.val_ptr);
			MathIConstant* test = static_cast<MathIConstant*>(obj.val_ptr);

			return dest->value == test->value;
		}

		return _Obj.name == obj.name;
	});
	
	if (it == m_Objects.end()) {
		m_Objects.push_back(_Obj);
		return m_Objects.size() - 1;
	}
	else
		return std::distance(m_Objects.begin(), it);
}

size_t MathI::get_object_index_by_name(const std::string& _Name) {
	auto it = std::find_if(m_Objects.begin(), m_Objects.end(), [&_Name](const MathIObject& obj) {
		return _Name == obj.name;
	});
	
	if (it == m_Objects.end()) {
		m_Objects.push_back(MathIObject::make_named_object(_Name));
		return m_Objects.size() - 1;
	}
	else
		return std::distance(m_Objects.begin(), it);
}

void MathI::generate_ast(const std::vector<Token>& _Tokens) {
	std::vector<AST*> ast;

	for (auto& token : _Tokens)
		ast.push_back(new AST(token));

	while (ast.size() != 1) {
		std::pair<size_t, size_t> parse_range = get_parse_range(ast);
		generate_range_ast(ast, parse_range); 
	}

	m_AST = ast.at(0);
}
 
void MathI::r_gen_opcode(std::vector<Opcode>& opcode, AST* ast, const std::vector<std::string>& params) {
	Token& tok = ast->token;

	if (tok.type == TokenType::expr_par) {
		ast = ast->nodes.at(0);
		tok = ast->token;
	}

	if (tok.type == TokenType::stmt_func_decl) {
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
					m_ErrorManager.codegen_error(
						m_CurrentSrc,
						"function declaration has a similar parameter name",
						arg->token.start, arg->token.end
					);

					return;
				}
			}

			args.push_back(arg->token.value);
		}

		MathiIFunction* function = new MathiIFunction;
		function->number_of_params = args.size();

		r_gen_opcode(function->opcode, ast->nodes.at(1), args);
		function->opcode.push_back(Opcode::halt);

		std::cout << "\n# <Function '" << tok.value << "'>:\n";
		debug_print_opcode(function->opcode);
		std::cout << '\n';

		opcode.push_back(Opcode::push_const);
		opcode.push_back((Opcode)(size_t)function);

		opcode.push_back(Opcode::make_func);
		opcode.push_back((Opcode)get_object_index_by_name(tok.value));
	}
	else if (tok.type == expr_num_const) {
		opcode.push_back(Opcode::push);

		double val = std::stod(std::string(tok.value));

		opcode.push_back((Opcode)get_object_index(
			MathIObject::make_const(val)
		));
	}
	else if (tok.type == expr_var) {
		bool added = false;

		if (!params.empty()) {
			std::string& name = tok.value;

			for (size_t i = 0; i < params.size(); ++i) {
				if (name == params.at(i)) {
					opcode.push_back(Opcode::lfs);
					opcode.push_back((Opcode)i);
					added = true;
					break;
				}
			}
		}

		if (!added) {
			opcode.push_back(Opcode::push);
			opcode.push_back((Opcode)get_object_index_by_name(tok.value));
		}
	}
	else if (tok.type == bin_op_assign) {
		r_gen_opcode(opcode, ast->nodes[1], params);

		opcode.push_back(Opcode::store);

		Token& dest_tok = ast->nodes[0]->token;
		opcode.push_back((Opcode)get_object_index_by_name(dest_tok.value)); 
	}
	else if (tok.type >= bin_op_pow && tok.type <= bool_bin_op_or) {
		r_gen_opcode(opcode, ast->nodes[0], params);
		r_gen_opcode(opcode, ast->nodes[1], params);

		opcode.push_back(Opcode::bin_op);
		opcode.push_back((Opcode)tok.type);
	}
	else if (tok.type >= bool_un_op_not && tok.type <= un_op_sub) {
		r_gen_opcode(opcode, ast->nodes[0], params);

		opcode.push_back(Opcode::un_op);
		opcode.push_back((Opcode)tok.type);
	}
	else if (tok.type == expr_func_call) {
		/*
								[expr_func_call AST node]
				expr_func_call					
					   |						
					  (,)					expr_func_call
					  / \			OR		       |
					 /   \						  expr
				   expr  (,)
				         / \
						/   \
					  expr  ...
		*/

		std::vector<AST*>& call_params = ast->nodes[0]->nodes; // [0] ->nodes;
		bool added = false;

		if (call_params.at(0)->token.type == expr_enum) {
			call_params = call_params[0]->nodes;
		}

		for (auto& node : call_params) {
			r_gen_opcode(opcode, node, params);
		}

		if (!added) {
			if (!params.empty()) {
				std::string& name = tok.value;

				for (size_t i = 0; i < params.size(); ++i) {
					if (name == params.at(i)) {
						opcode.push_back(Opcode::lfs);
						opcode.push_back((Opcode)i);
						added = true; 
						break;
					}
				}
			}

			if (!added) {
				size_t function_index = get_object_index_by_name(tok.value);
				opcode.push_back(Opcode::push);
				opcode.push_back((Opcode)function_index);
				added = true;
			}

			opcode.push_back(Opcode::call);
			opcode.push_back((Opcode)call_params.size());
		}
	}
}

void MathI::debug_print_opcode(const std::vector<Opcode>& _Opcode) {
	for (size_t i = 0; i < _Opcode.size(); ++i) {

		if (_Opcode.at(i) == Opcode::push) {
			auto& obj = m_Objects.at((size_t)_Opcode.at(i + 1));

			printf("%d:\tpush\t%d\t\t(", 
				(int)i, 
				(int)_Opcode.at(i + 1)
			);

			if (obj.val_ptr == nullptr) {
				printf("object '%s')\n", obj.name.data());
			}
			else if (obj.constant) {
				printf("const %f)\n", static_cast<MathIConstant*>(obj.val_ptr)->value);
			}
			else if (obj.callable)
				printf("func '%s' at 0x%p)\n", obj.name.data(), obj.val_ptr);
			else {
				auto v = static_cast<MathIVariable*>(obj.val_ptr);

				if (v == nullptr || !v->initialized)
					printf("var %s=???)\n", obj.name.data());
				else
					printf("var %s=%f)\n", obj.name.data(), static_cast<MathIVariable*>(obj.val_ptr)->value);
			}

			i += 1;
		}

		else if (_Opcode.at(i) == Opcode::lfs) {
			printf("%llu:\tlfs\t%llu+offset\n",
				i,
				_Opcode.at(i + 1)
			);

			i += 1;
		}

		else if (_Opcode.at(i) == Opcode::push_const) {
			printf("%llu:\tpush_const\t0x%p\n",
				i,
				(void*)(size_t)_Opcode.at(i + 1)
			);

			i += 1;
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

			printf("%d:\tcall\t%llu\n", 
				(int)i, 
				_Opcode.at(i + 1)
			);
			i += 1;
		}

		else if (_Opcode.at(i) == Opcode::make_func) {
			printf("%llu:\tmake_func\t%llu\n",
				i,
				(size_t)_Opcode.at(i + 1)
			);

			i += 1;
		}

		else if (_Opcode.at(i) == Opcode::store) {
			auto& obj = m_Objects.at((size_t)_Opcode.at(i + 1));

			printf("%llu:\tstore\t%llu\t\t(",
				i,
				(size_t)_Opcode.at(i + 1)
			);

			if (obj.val_ptr == nullptr) {
				printf("object '%s')\n", obj.name.data());
			}
			else if (obj.constant) {
				printf("const %f)\n", static_cast<MathIConstant*>(obj.val_ptr)->value);
			}
			else if (obj.callable)
				printf("func '%s' at 0x%p)\n", obj.name.data(), obj.val_ptr);
			else {
				auto v = static_cast<MathIVariable*>(obj.val_ptr);

				if (v == nullptr || !v->initialized)
					printf("var %s=???)\n", obj.name.data());
				else
					printf("var %s=%f)\n", obj.name.data(), static_cast<MathIVariable*>(obj.val_ptr)->value);
			}

			i += 1;
		}

		else if (_Opcode.at(i) == Opcode::halt) {
			printf("%d:\thalt\n", (int)i);
			break;
		}
	}
}

void MathI::gen_executable() {
	opcode.clear();
	r_gen_opcode(opcode, m_AST, std::vector<std::string>());
	opcode.push_back(Opcode::halt);
}

MathIObject* MathI::execute(std::vector<Opcode>& _Opcode) {
	MathIObject* objects = m_Objects.data();
	Opcode* instructions = _Opcode.data();
	size_t i = 0;

	while (instructions[i] != Opcode::halt) {
		Opcode inst0 = instructions[i];
		Opcode inst1 = instructions[i + 1];

		switch (inst0)
		{
		case Opcode::push: {
			if (stack_counter >= max_stack_length) {
				m_ErrorManager.runtime_error(
					m_CurrentSrc,
					"stack overflow",
					0, m_CurrentSrc.size() - 1
				);
				return nullptr;
			}

			MathIObject* tmp = &objects[(size_t)inst1];
			std::string s = get_mathi_object_info(tmp);

			auto r = offset_stack[offset_stack_counter - 1];

			stack[stack_counter] = &objects[(size_t)inst1];
			stack_counter += 1;
			i += 1; 
			break;
		}

		case Opcode::push_const: {
			if (stack_counter >= max_stack_length) {
				m_ErrorManager.runtime_error(
					m_CurrentSrc,
					"stack overflow",
					0, m_CurrentSrc.size() - 1
				);
				return nullptr;
			}

			stack[stack_counter] = (MathIObject*)inst1;
			stack_counter += 1;
			i += 1;
			break;
		}

		case Opcode::bin_op: {
			MathIObject& left_obj = *stack[stack_counter - 2];
			MathIObject& right_obj = *stack[stack_counter - 1];
			stack_counter -= 1;

			if (left_obj.callable || right_obj.callable) {
				m_ErrorManager.runtime_error(
					m_CurrentSrc,
					"it is not possible to perform a binary operation with the called object",
					0, m_CurrentSrc.size() - 1
				);
				return nullptr;
			}

			double left_val, right_val;

			if (left_obj.constant) {
				left_val = static_cast<MathIConstant*>(left_obj.val_ptr)->value;
			}
			else {
				auto left_mathi_obj = static_cast<MathIVariable*>(left_obj.val_ptr);

				if (left_mathi_obj == nullptr || !left_mathi_obj->initialized) {
					m_ErrorManager.runtime_error(
						m_CurrentSrc,
						"the object has not been initialized",
						0, m_CurrentSrc.size() - 1
					);
					return nullptr;
				}

				left_val = static_cast<MathIVariable*>(left_obj.val_ptr)->value;
			}

			if (right_obj.constant) {
				right_val = static_cast<MathIConstant*>(right_obj.val_ptr)->value;
			}
			else {
				auto right_mathi_obj = static_cast<MathIVariable*>(right_obj.val_ptr);

				if (right_mathi_obj == nullptr || !right_mathi_obj->initialized) {
					m_ErrorManager.runtime_error(
						m_CurrentSrc,
						"the object has not been initialized",
						0, m_CurrentSrc.size() - 1
					);
					return nullptr;
				}

				right_val = static_cast<MathIVariable*>(right_obj.val_ptr)->value;
			}

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

			MathIConstant* new_const = new MathIConstant{ res };
			MathIObject* new_obj = new MathIObject{
				false, true, "", (void*)new_const
			};

			stack[stack_counter - 1] = new_obj;
			i += 1;
			break;
		}
		case Opcode::un_op: {
			MathIObject& obj = *stack[stack_counter - 1];
			double res = 0.0;

			double obj_val;

			if (obj.constant) {
				obj_val = static_cast<MathIConstant*>(obj.val_ptr)->value;
			}
			else {
				auto left_mathi_obj = static_cast<MathIVariable*>(obj.val_ptr);

				if (left_mathi_obj == nullptr || !left_mathi_obj->initialized) {
					m_ErrorManager.runtime_error(
						m_CurrentSrc,
						"the object has not been initialized",
						0, m_CurrentSrc.size() - 1
					);
					return nullptr;
				}

				obj_val = static_cast<MathIVariable*>(obj.val_ptr)->value;
			}

			switch (inst1) {
			case Opcode::uo_neg:
				res = -obj_val;
				break;
			case Opcode::uo_not:
				res = (double)(!((bool)obj_val));
				break;
			}

			MathIConstant* new_const = new MathIConstant{ res };
			MathIObject* new_obj = new MathIObject{
				false, true, "", (void*)new_const
			};

			stack[stack_counter - 1] = new_obj;

			i += 1;

			break;
		}
		case Opcode::call: {
			size_t arg_count = (size_t)inst1;
			size_t stack_frame = stack_counter - arg_count;

			MathIObject& target_object = *stack[stack_counter - 1];

			if (!target_object.callable) {
				m_ErrorManager.runtime_error(
					m_CurrentSrc,
					"the objects is not callable",
					0, m_CurrentSrc.size() - 1
				);
				return nullptr;
			}

			if (target_object.val_ptr == nullptr) {
				m_ErrorManager.runtime_error(
					m_CurrentSrc,
					"the object has not been initialized",
					0, m_CurrentSrc.size() - 1
				);
				return nullptr;
			}

			if (target_object.val_ptr == nullptr) {
				m_ErrorManager.runtime_error(
					m_CurrentSrc,
					"function has not been initialized",
					0, m_CurrentSrc.size() - 1
				);
				return nullptr;
			}

			MathiIFunction& function = *(MathiIFunction*)target_object.val_ptr;

			if (function.number_of_params != arg_count) {
				m_ErrorManager.runtime_error(
					m_CurrentSrc,
					std::format("{} arguments are passed to the function instead of {}", arg_count, function.number_of_params),
					0, m_CurrentSrc.size() - 1
				);
				return nullptr;
			}

			size_t new_offset = stack_counter - function.number_of_params - 1;
			offset_stack[offset_stack_counter] = new_offset;
			offset_stack_counter += 1;
			
			auto res = execute(function.opcode);
			stack_counter = stack_frame;

			stack[stack_frame - 1] = res;
			offset_stack_counter -= 1;
			i += 1;

			break;
		}

		case Opcode::lfs: {
			size_t offset = offset_stack[offset_stack_counter - 1];

			stack[stack_counter] = stack[offset + (size_t)inst1];
			stack_counter += 1;
			i += 1;
			break;
		}
		
		case Opcode::store: {
			MathIObject& target = *stack[stack_counter - 1];
			
			size_t index = (size_t)inst1;

			if (target.callable) {
				objects[index].callable = true;
				objects[index].constant = false;
				objects[index].val_ptr = target.val_ptr;
			}
			else if (target.constant) {
				objects[index].callable = false;
				objects[index].constant = false;

				MathIConstant* target_const = (MathIConstant*)target.val_ptr;
				MathIVariable* new_const = new MathIVariable;
				new_const->initialized = true;
				new_const->value = target_const->value;

				objects[index].val_ptr = new_const;
			}
			else {
				objects[index].callable = false;
				objects[index].constant = false;

				MathIVariable* target_const = (MathIVariable*)target.val_ptr;

				if (target_const == nullptr || !target_const->initialized) {
					m_ErrorManager.runtime_error(
						m_CurrentSrc,
						"the object has not been initialized",
						0, m_CurrentSrc.size() - 1
					);
					return nullptr;
				}

				MathIVariable* new_var = new MathIVariable;
				new_var->initialized = true;
				new_var->value = target_const->value;

				objects[index].val_ptr = new_var; 
			}

			i += 1; 

			break;
		}

		case Opcode::make_func: {
			MathIObject& target = objects[(size_t)inst1];
			target.callable = true;
			target.constant = false;
			target.val_ptr = (void*)stack[stack_counter - 1];
			stack_counter -= 1;

			i += 1;

			break;
		}
		case Opcode::halt:
			break;
		default:
			break;
		}

		i += 1;
	}

	/*std::cout << "STACK: { ";
	for (size_t i = 0; i < stack_counter; ++i) {
		std::cout << get_mathi_object_info(stack[i]) << ' ';
	}
	std::cout << "<-- top }\n\n";

	std::cout << "OFFSET_STACK: { ";
	for (size_t i = 0; i < offset_stack_counter; ++i) {
		std::cout << i << ' ';
	}
	std::cout << "<-- top }\n\n";*/

	if (stack_counter == 0)
		return nullptr;

	return stack[stack_counter - 1];
}

static const char* debug_token_type_to_str(TokenType tt) {
	switch (tt) {
	case nothing: return "nothing";
	case sym_eos: return "sym_eos";
	case sym_comma: return "sym_comma";
	case sym_semicolon: return "sym_semicolon";
	case sym_lpar: return "sym_lpar";
	case sym_rpar: return "sym_rpar";
	case bin_op_pow: return "bin_op_pow";
	case bin_op_mul: return "bin_op_mul";
	case bin_op_div: return "bin_op_div";
	case bin_op_add: return "bin_op_add";
	case bin_op_sub: return "bin_op_sub";
	case bin_op_assign: return "bin_op_assign";
	case bool_bin_op_less: return "bool_bin_op_less";
	case bool_bin_op_more: return "bool_bin_op_more";
	case bool_bin_op_less_eq: return "bool_bin_op_less_eq";
	case bool_bin_op_more_eq: return "bool_bin_op_more_eq";
	case bool_bin_op_eq: return "bool_bin_op_eq";
	case bool_bin_op_not_eq: return "bool_bin_op_not_eq";
	case bool_bin_op_and: return "bool_bin_op_and";
	case bool_bin_op_or: return "bool_bin_op_or";
	case bool_un_op_not: return "bool_un_op_not";
	case un_op_sub: return "un_op_sub";
	case expr_num_const: return "expr_num_const";
	case expr_var: return "expr_var";
	case expr_un_op: return "expr_un_op";
	case expr_bin_op: return "expr_bin_op";
	case expr_par: return "expr_par";
	case expr_func_call: return "expr_func_call";
	case expr_cond: return "expr_cond";
	case expr_enum: return "expr_enum";
	case var_enum: return "var_enum";
	case expr: return "expr";
	case stmt_func_decl: return "stmt_func_decl";
	case stmt: return "stmt";
	case final_instruction: return "final_instruction";
	}
	return "<TokenType is unknown>";
}

static void debug_print_tokens(const std::vector<Token> tokens) {
	for (auto& tok : tokens)
		std::cout << std::format("[{} '{}' {}:{}] ", 
			debug_token_type_to_str(tok.type),
			tok.value,
			tok.start,
			tok.end
		);
	std::cout << '\n';
}

double MathI::eval(const std::string& _Str) {
	double result = 0.0;

	for (auto& range : divide_into_expr(_Str)) {
		clear_error();
		
		std::string tmp_src = _Str.substr(range.first, range.second - range.first);
		m_CurrentSrc = tmp_src;

		std::vector<Token> tokens;
		tokens.reserve(tmp_src.size());
		Tokenizer::parse(tmp_src, tokens, m_ErrorManager); 
		
		if (!ok()) return 0.0;

		// debug_print_tokens(tokens); 
		
		{
			std::vector<Token> token_array;
			token_array.resize(tokens.size());
			std::copy(tokens.begin(), tokens.end(), token_array.begin());

			rule(token_array, tmp_src, m_ErrorManager); // __debugbreak();
		}

		if (!ok()) return 0.0;

		generate_ast(tokens);//  __debugbreak();

		// print_ast(m_AST);

		stack_counter = 0;	
		
		gen_executable();//  __debugbreak();

		if (!ok()) return 0.0;

		debug_print_opcode(opcode); // __debugbreak();

		if (!ok()) return 0.0;

		MathIObject* obj_res = (MathIObject*)execute(opcode); 
		
		std::cout << "\nResult >> " << get_mathi_object_info(obj_res) << '\n';
	}

	return 0;
};

#undef CMATH_FUNC_1_ARG
#undef CMATH_FUNC_2_ARG
#undef CMATH_GET_FUNC_NAME