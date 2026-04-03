#include "parser.h"

void fill_token_type_array(TokenType* _Array, size_t _MaxLen, TokenTree* _Current) {
	TokenTree* cur = _Current;

	for (size_t i = 0; (i < _MaxLen) && (!cur->is_end()); ++i) {
		_Array[i] = cur->token.type;

		cur = cur->next;
	}
}
