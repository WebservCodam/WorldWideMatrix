#pragma once

#include "Configuration.hpp"

class	Lexer
{
	private:
		// std::vector<Token>	_tokens;
		// size_t				_line_num = 1;
		// size_t				_col_num = 1;

		static std::string	consumeNumber(const std::string& input, size_t& pos);
		static bool			isValidWordChar(char c);
		static std::string	consumeWord(const std::string& input, size_t& pos);
		static std::string	consumeString(const std::string& input, size_t& pos, size_t line, size_t col);

	public:
		static std::vector<Token>	tokenize(const std::string& input);
};
