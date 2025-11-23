#pragma once

#include "Configuration.hpp"

enum class ErrorType
{
	LEXER,
	PARSER,
	VALIDATOR
};

class ConfigError : public std::runtime_error
{
	private:
		ErrorType	_type;
		size_t		_line;
		size_t		_column;
		std::string	_context;

		static std::string	buildMessage(ErrorType type, const std::string& message,
									size_t line, size_t column, const std::string& context);

	public:
		ConfigError(ErrorType type, const std::string& message,
					size_t line, size_t column,
					const std::string& context = "");

		ConfigError(ErrorType type, const std::string& message,
					const Directive* directive);

		static ConfigError	parsing(const std::string& message, size_t line, size_t column);
		static ConfigError	validation(const std::string& message, const Directive* directive);

		ErrorType			getType() const { return _type; }
		size_t				getLine() const { return _line; }
		size_t				getColumn() const { return _column; }
		const std::string&	getContext() const { return _context; }

		const char*			what() const noexcept override;
};
