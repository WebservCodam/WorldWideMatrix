#pragma once

#include <Configuration.hpp>

class	Lexer
{
	private:
		std::string	consumeNumber(const std::string& input, size_t& pos);
    	bool		isValidWordChar(char c);
    	std::string	consumeWord(const std::string& input, size_t& pos);

	public:
		std::vector<Token>	tokenize(const std::string& input);
};
