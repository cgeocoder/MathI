#include "errors.h"

std::string g_error_message;

void __mathi_make_error(MathIError&& _Error) {
	g_error_message = _Error.get_error();
}

std::string __mathi_get_error() {
	return g_error_message;
}

bool __mathi_is_ok() {
	return g_error_message.empty();
}

void __mathi_clear_error() {
	g_error_message.clear();
}