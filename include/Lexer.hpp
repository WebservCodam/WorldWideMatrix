#pragma once

#include <Configuration.hpp>

class	Lexer
{
	private:
		static std::string	consumeNumber(const std::string& input, size_t& pos);
		static bool			isValidWordChar(char c);
		static std::string	consumeWord(const std::string& input, size_t& pos);

	public:
		static std::vector<Token>	tokenize(const std::string& input);
};
