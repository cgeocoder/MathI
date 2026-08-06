#pragma once

#ifndef __MATHI_ERRORS_H__
#define __MATHI_ERRORS_H__

#include <string>
#include <format>
#include <iostream>
#include <stdexcept>
#include <queue>
#include <list>

/*
	Types of error

	1. Parse error
	2. Syntax error
	3. Code generation error
	4. Runtime error
*/

class MathIError {
public:
	const std::string source;
	const std::string description;
	const std::string type_of_error;

	MathIError(const std::string& _ErrType, const std::string& _Src, const std::string& _Desc, size_t _Start, size_t _End)
		: type_of_error{ _ErrType }, source{ _Src }, description{ _Desc }, p_Start{ _Start }, p_End{ _End }{}

	virtual ~MathIError() = default;

	inline std::string get_error() {
		return std::format(
			"mathi: {} error: {}\nmathi: {}\n{}{}\n\n",
			type_of_error, description, source,
			std::string(p_Start + 7, ' '),
			std::string(p_End - p_Start, '^')
		);
	}
	
protected:
	const size_t p_Start, p_End;
};

class MathIErrorManager {
private:
	std::list<std::unique_ptr<MathIError>> m_Errors;

public:	
	inline MathIErrorManager() {}
	inline virtual ~MathIErrorManager() = default;

	inline bool ok() const noexcept { return m_Errors.empty(); }
	inline void clear() { m_Errors.clear(); }

	inline std::string get_last_error() {
		std::string err = m_Errors.back()->get_error();
		m_Errors.pop_back();
		return err;
	}
	
	inline void parser_error(const std::string& _Src, const std::string& _Desc, size_t _Start, size_t _End) {
		m_Errors.push_back(std::make_unique<MathIError>(
			"parse", _Src, _Desc, _Start, _End
		));
	}

	inline void syntax_error(const std::string& _Src, const std::string& _Desc, size_t _Start, size_t _End) {
		m_Errors.push_back(std::move(std::make_unique<MathIError>(
			"syntax", _Src, _Desc, _Start, _End
		)));
	}

	inline void codegen_error(const std::string& _Src, const std::string& _Desc, size_t _Start, size_t _End) {
		m_Errors.push_back(std::move(std::make_unique<MathIError>(
			"code gen", _Src, _Desc, _Start, _End
		)));
	}
	
	inline void runtime_error(const std::string& _Src, const std::string& _Desc, size_t _Start, size_t _End) {
		m_Errors.push_back(std::move(std::make_unique<MathIError>(
			"runtime", _Src, _Desc, _Start, _End
		)));
	}
};

#endif // !__MATHI_ERRORS_H__