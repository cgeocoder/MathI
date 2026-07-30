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
	inline MathIError(
		const std::string_view& _Src,
		const std::string_view& _Desc,
		size_t _Start,
		size_t _End
	)
		: source{ _Src }, description{ _Desc }, 
		start{ _Start }, end{ _End } {}

	virtual ~MathIError() = default;

protected:
	size_t start, end;
	std::string_view description, source;

	inline std::string error_tag(const std::string_view& _Tag) const {
		return std::format(
			"mathi: error: {}\nmathi: {}\n{}{}\n\n",
			_Tag, description, std::string(start + 6, ' '), std::string(end - start, '^')
		);
	}
};

class MathIParseError : public MathIError {
public:
	inline MathIParseError(const std::string_view& _Src,const std::string_view& _Desc,size_t _Start,size_t _End) noexcept
		: MathIError(_Src, _Desc, _Start, _End) {}

	virtual ~MathIParseError() noexcept = default;

	virtual const char* what() const noexcept {
		return error_tag("parse").c_str();
	}
};

class MathISyntaxError : public MathIError {
public:
	inline MathISyntaxError(const std::string_view& _Src, const std::string_view& _Desc, size_t _Start, size_t _End) noexcept
		: MathIError(_Src, _Desc, _Start, _End) {}
		
	virtual ~MathISyntaxError() noexcept  = default;

	virtual const char* what() const noexcept {
		return error_tag("syntax").c_str();
	}
};

class MathICodeGenError : public MathIError {
public:
	inline MathICodeGenError(const std::string_view& _Src, const std::string_view& _Desc, size_t _Start, size_t _End) noexcept
		: MathIError(_Src, _Desc, _Start, _End) {}

	virtual ~MathICodeGenError() noexcept = default;

	virtual const char* what() const noexcept {
		return error_tag("code gen").c_str();
	}
};

class MathIRuntimeError : public MathIError {
public:
	inline MathIRuntimeError(const std::string_view& _Src, const std::string_view& _Desc, size_t _Start, size_t _End) noexcept
		: MathIError(_Src, _Desc, _Start, _End) {}

	virtual ~MathIRuntimeError() noexcept = default;

	virtual const char* what() const noexcept {
		return error_tag("runtime").c_str();
	}
};
