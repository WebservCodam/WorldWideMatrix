#pragma once

#include "Configuration.hpp"

class ParseError : public std::runtime_error
{
	private:
		size_t	_line;
		size_t	_column;

	public:
		ParseError(const std::string& message, size_t line, size_t column);

		size_t	getLine() const { return _line; }
		size_t	getColumn() const { return _column; }

		const char *what() const noexcept override;
};
