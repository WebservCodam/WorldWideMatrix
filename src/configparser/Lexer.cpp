#include "Configuration.hpp"

void		Lexer::advancePosition(int len, size_t& pos)
{
	this->_col_num += len;
	pos += len;
}

bool	Lexer::isValidWordChar(char c)
{
	if (std::isspace(c) || c == '{' || c == '}' || c == ';'|| c == ',' || c == '#')
		return (false);
	return (true);
}

std::string	Lexer::consumeWord(const std::string& input, size_t& pos)
{
	std::string	word;

	while (pos < input.length() && isValidWordChar(input[pos]))
	{
		word += input[pos];
		advancePosition(1, pos);
	}
	return (word);
}

std::string	Lexer::consumeString(const std::string& input, size_t& pos, size_t line, size_t col)
{
	std::string	string;
	char		quote = input[pos];	// Store opening quote

	advancePosition(1, pos);		// Skip opening quote
	while (pos < input.length() && input[pos] != quote)
	{
		string += input[pos];
		advancePosition(1, pos);
	}

	if (pos >= input.length())
		throw ConfigError(ErrorType::LEXER, "Unterminated string literal", line, col);

	advancePosition(1, pos);		// Skip closing quote
	return (string);
}


std::vector<Token>	Lexer::tokenize()
{
	size_t	pos = 0;

	while (pos < _input.length())
	{
		char	current_char = _input[pos];

		//	--- Rule 1: Handle Whitespace
		if (isspace(current_char))
		{
			if (current_char == '\n')
			{
				_line_num++;
				_col_num = 1;
			}
			else
				_col_num++;
			pos++;	// Consume the whitespace and continue to the next loop iteration.
			continue;
		}

		//	--- Rule 2: Handle Comments
		else if (current_char == '#')
		{
			while (pos < _input.length() && _input[pos] != '\n')
				pos++;
			continue ;
		}

		//	--- Rule 3: Handle Single-Character Tokens
		else if (current_char == '{')
		{
			_tokens.push_back({LBRACE, "{", _line_num, _col_num}); // Replace with addToken(LBRACE, "{");
			advancePosition(1, pos);
			continue ;
		}

		else if (current_char == '}')
		{
			_tokens.push_back({RBRACE, "}", _line_num, _col_num});
			advancePosition(1, pos);
			continue ;
		}

		else if (current_char == ';')
		{
			_tokens.push_back({SEMICOLON, ";", _line_num, _col_num});
			advancePosition(1, pos);
			continue ;
		}

		else if (current_char == ',')
		{
			_tokens.push_back({COMMA, ",", _line_num, _col_num});
			advancePosition(1, pos);
			continue ;
		}

		//	--- Rule 4: Handle Strings
		else if (current_char == '"' || current_char == '\'')
		{
			std::string stringValue = consumeString(_input, pos, _line_num, _col_num);
			_tokens.push_back({STRING, stringValue, _line_num, _col_num});
			continue ;
		}

		//	--- Rule 5: Handle Words
		else if (isValidWordChar(current_char))
		{
			std::string	wordValue = consumeWord(_input, pos);
			_tokens.push_back({WORD, wordValue, _line_num, _col_num});
		}

		//	--- Rule 6: Handle Errors
		else
		{
			throw ConfigError(ErrorType::LEXER,
							"Unexpected character '" + std::string(1, current_char) + "'",
							_line_num, _col_num);
			pos++;	// Just advance to avoid an infinite loop
		}
	}
	_tokens.push_back({END_OF_FILE, "", _line_num, _col_num});
	return (this->_tokens);
}
