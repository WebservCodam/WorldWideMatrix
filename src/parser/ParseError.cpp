#include "ParseError.hpp"

ParseError::ParseError(const std::string& message, size_t line, size_t column) :
std::runtime_error("Parse error at line " + std::to_string(line)
					+ ", column " + std::to_string(column) + ": " + message),
					_line(line), _column(column) {}  // Initializer list

const char* ParseError::what() const noexcept {
	return std::runtime_error::what();
}
