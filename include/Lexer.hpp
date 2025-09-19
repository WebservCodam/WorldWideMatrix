#pragma once

#include <Configuration.hpp>

class	Lexer
{
	public:
		std::vector<Token>	tokenize(const std::string& input);
};
