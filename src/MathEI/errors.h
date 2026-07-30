#pragma once

#ifndef __MATHI_ERRORS_H__
#define __MATHI_ERRORS_H__

#include <string>
#include <format>
#include <iostream>
#include <stdexcept>

/*
	Types of error

	1. Parse error
	2. Syntax error
	3. Code generation error
	4. Runtime error
*/

class MathIError : public std::exception {
public:
	virtual ~MathIError() = default;

	const char* what() const override {
		return err_msg.c_str();
	}

protected:
	std::string err_msg;

	inline void make_error(
		const std::string_view& _Tag,
		const std::string_view& _Desc,
		const std::string_view& _Src,
		size_t _Start,
		size_t _End
	) {
		err_msg = std::format(
			"mathi: {} error: {}\nmathi: {}\n{}{}\n\n",
			_Tag, _Desc, _Src,
			std::string(_Start + 7, ' '), 
			std::string(_End - _Start + 1, '^')
		);
	}
};

class MathIParseError : public MathIError {
public:
	inline MathIParseError(const std::string_view& _Src, const std::string_view& _Desc, size_t _Start, size_t _End) noexcept
	{
		make_error("parse", _Desc, _Src, _Start, _End);
	}

	virtual ~MathIParseError() noexcept = default;
};

class MathISyntaxError : public MathIError {
public:
	inline MathISyntaxError(const std::string_view& _Src, const std::string_view& _Desc, size_t _Start, size_t _End) noexcept
	{
		make_error("syntax", _Desc, _Src, _Start, _End);
	}
		
	virtual ~MathISyntaxError() noexcept  = default;
};

class MathICodeGenError : public MathIError {
public:
	inline MathICodeGenError(const std::string_view& _Src, const std::string_view& _Desc, size_t _Start, size_t _End) noexcept
	{
		make_error("code gen", _Desc, _Src, _Start, _End);
	}

	virtual ~MathICodeGenError() noexcept = default;
};

class MathIRuntimeError : public MathIError {
public:
	inline MathIRuntimeError(const std::string_view& _Src, const std::string_view& _Desc, size_t _Start, size_t _End) noexcept
	{
		make_error("runtime", _Desc, _Src, _Start, _End);
	}

	virtual ~MathIRuntimeError() noexcept = default;
};

#endif // !__MATHI_ERRORS_H__