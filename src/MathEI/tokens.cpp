#include "tokens.h"
#include "errors.h"

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

void Tokenizer::parse(const std::string& _Str, std::vector<Token>& _Tokens) {
    const auto start = _Str.begin();
    auto tmp_start = _Str.begin(), end = _Str.end();

    while (tmp_start != end) {
        if (std::isalpha(*tmp_start)) {
            auto tmp_end = std::find_if(tmp_start, end, [](const char ch) -> bool {
                return !(std::isalpha(ch) || std::isdigit(ch) || ch == '_');
            });

            size_t sub_start = std::distance(start, tmp_start),
                sub_end = std::distance(start, tmp_end);

            _Tokens.push_back({
                [&]() -> TokenType {
                    std::string tok_str(tmp_start, tmp_end);
                    if (tok_str == "and") return TokenType::bool_bin_op_and;
                    if (tok_str == "or") return TokenType::bool_bin_op_or;
                    if (tok_str == "not") return TokenType::bool_un_op_not;
                    return TokenType::expr_var;
                }(),
                _Str.substr(sub_start, sub_end - sub_start),
                sub_start, sub_end
            });

            tmp_start = tmp_end; 
        }

        else if (std::isdigit(*tmp_start)) {
            bool is_floating = false, invalid_float = false;
            auto tmp_end = std::find_if(tmp_start, end, [&](const char ch) -> bool {
                if (ch == '.') {
                    if (is_floating)
                        return !(invalid_float = true);

                    return !(is_floating = true);
                }

                return !std::isdigit(ch);
            });

            size_t sub_start = std::distance(start, tmp_start),
                sub_end = std::distance(start, tmp_end);

            if (invalid_float) {
                __mathi_make_error(MathIParseError(
                    _Str, "invalid floating number",
                    sub_start, sub_end
                ));
                return;
            }

            _Tokens.push_back({
                TokenType::expr_num_const,
                _Str.substr(sub_start, sub_end - sub_start),
                sub_start, sub_end
            });

            tmp_start = tmp_end;
        }
        else if (std::next(tmp_start) != end && is_long_spec_symbol(*tmp_start, *std::next(tmp_start))) {
            size_t sub_start = std::distance(start, tmp_start),
                sub_end = std::distance(start, std::next(tmp_start, 2));

            _Tokens.push_back({
                [&]() -> TokenType {
                    char ch1 = *tmp_start;
                    if (ch1 == '<') return TokenType::bool_bin_op_less_eq;
                    if (ch1 == '>') return TokenType::bool_bin_op_more_eq;
                    if (ch1 == '=') return TokenType::bool_bin_op_eq;
                    if (ch1 == '!') return TokenType::bool_bin_op_not_eq;
                    return TokenType::nothing;
                }(),
                _Str.substr(sub_start, sub_end - sub_start),
                sub_start, sub_end
            });
            tmp_start += 2;
        }
        else if (is_spec_symbol(*tmp_start)) {
            size_t sub_start = std::distance(start, tmp_start),
                sub_end = std::distance(start, std::next(tmp_start));

            _Tokens.push_back({
                [&]() -> TokenType {
                    switch (*tmp_start) {
                    case ',': return TokenType::sym_comma;
                    case ';': return TokenType::sym_semicolon;
                    case '(': return TokenType::sym_lpar;
                    case ')': return TokenType::sym_rpar;
                    case '^': return TokenType::bin_op_pow;
                    case '*': return TokenType::bin_op_mul;
                    case '/': return TokenType::bin_op_div;
                    case '+': return TokenType::bin_op_add;
                    case '=': return TokenType::bin_op_assign;
                    case '<': return TokenType::bool_bin_op_less;
                    case '>': return TokenType::bool_bin_op_more;
                    case '-': {
                        if (_Tokens.size() == 0)
                            return TokenType::un_op_sub;
                        
                        auto prev_type = std::prev(_Tokens.end())->type;

                        if (prev_type != expr_num_const && prev_type != expr_var && prev_type != sym_rpar)
                            return TokenType::un_op_sub;

                        return TokenType::bin_op_sub;
                    }
                    default: return TokenType::nothing;
                    }
                }(),
                _Str.substr(sub_start, sub_end - sub_start),
                sub_start, sub_end
            });
            ++tmp_start;
        }
        else if (char ch = *tmp_start; ch == ' ' or ch == '\t' or ch == '\n') {
            ++tmp_start;
        }
        else {
            __mathi_make_error(MathIParseError(
                _Str, std::format("invalid symbol '{}'", *tmp_start),
                std::distance(start, tmp_start),
                std::distance(start, std::next(tmp_start))
            ));
            return;
        }
    }
}