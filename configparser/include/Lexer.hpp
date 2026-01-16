#pragma once

#include "Configuration.hpp"

class	Lexer
{
	private:
		std::string			_input;
		std::vector<Token>	_tokens;
		size_t				_line_num = 1;
		size_t				_col_num = 1;

		std::string	consumeNumber(const std::string& input, size_t& pos);
		bool		isValidWordChar(char c);
		std::string	consumeWord(const std::string& input, size_t& pos);
		std::string	consumeString(const std::string& input, size_t& pos, size_t line, size_t col);

	public:
		Lexer() = delete; // Can default to a config file.
		Lexer(const std::string& input);
		~Lexer();

		void	tokenize();

		std::vector<Token>	getTokens();
};
