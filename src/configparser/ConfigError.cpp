#include "Configuration.hpp"

// ----- CONSTRUCTORS -----

ConfigError::ConfigError(ErrorType type, const std::string& message):
	std::runtime_error(buildMessage(type, message)), _type(type) {}

ConfigError::ConfigError(ErrorType type, const std::string& message, size_t line, size_t column, const std::string& context):
	std::runtime_error(buildMessage(type, message, line, column, context)), _type(type), _line(line), _column(column), _context(context) {}

ConfigError::ConfigError(ErrorType type, const std::string& message, const Directive* directive)
	: ConfigError(type, message, directive->getLine(), directive->getColumn(), directive->getName()) {}

std::string ConfigError::buildMessage(ErrorType type, const std::string& message)
{
	std::string typeStr;
	switch(type)
	{
		case ErrorType::INITIALIZATION:
			typeStr = "Initialization";
			break;
		case ErrorType::LEXER:
			typeStr = "Lexer";
			break;
		case ErrorType::PARSER:
			typeStr = "Parse";
			break;
		case ErrorType::VALIDATOR:
			typeStr = "Validation";
			break;
	}

	std::string msg = typeStr + " error: " + message;
	return (msg);
}

std::string ConfigError::buildMessage(ErrorType type, const std::string& message, size_t line, size_t column, const std::string& context)
{
	std::string typeStr;
	switch(type)
	{
		case ErrorType::INITIALIZATION:
			typeStr = "Initialization";
			break;
		case ErrorType::LEXER:
			typeStr = "Lexing";
			break;
		case ErrorType::PARSER:
			typeStr = "Parsing";
			break;
		case ErrorType::VALIDATOR:
			typeStr = "Validation";
			break;
	}

	std::string msg = typeStr + " error at line " + std::to_string(line)
					+ ", column " + std::to_string(column);

	if (!context.empty())
		msg += " (" + context + ")";

	msg += ": " + message;

	return (msg);
}

ConfigError ConfigError::initialization(const std::string& message)
{
	return ConfigError(ErrorType::INITIALIZATION, message);
}

ConfigError ConfigError::lexing(const std::string& message, size_t line, size_t column)
{
	return ConfigError(ErrorType::LEXER, message);
}

ConfigError ConfigError::parsing(const std::string& message, size_t line, size_t column)
{
	return ConfigError(ErrorType::PARSER, message, line, column);
}

ConfigError ConfigError::validation(const std::string& message, const Directive* directive)
{
	return ConfigError(ErrorType::VALIDATOR, message, directive);
}

const char* ConfigError::what() const noexcept
{
	return std::runtime_error::what();
}
