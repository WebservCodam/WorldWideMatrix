#include "Configuration.hpp"

void		Lexer::advancePosition(int len, size_t& pos)
{
	this->_colNum += len;
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
		char	currentChar = _input[pos];

		//	--- Rule 1: Handle Whitespace
		if (isspace(currentChar))
		{
			if (currentChar == '\n')
			{
				_lineNum++;
				_colNum = 1;
			}
			else
				_colNum++;
			pos++;	// Consume the whitespace and continue to the next loop iteration.
			continue;
		}

		//	--- Rule 2: Handle Comments
		else if (currentChar == '#')
		{
			while (pos < _input.length() && _input[pos] != '\n')
				pos++;
			continue ;
		}

		//	--- Rule 3: Handle Single-Character Tokens
		else if (currentChar == '{')
		{
			_tokens.push_back({LBRACE, "{", _lineNum, _colNum});
			advancePosition(1, pos);
			continue ;
		}

		else if (currentChar == '}')
		{
			_tokens.push_back({RBRACE, "}", _lineNum, _colNum});
			advancePosition(1, pos);
			continue ;
		}

		else if (currentChar == ';')
		{
			_tokens.push_back({SEMICOLON, ";", _lineNum, _colNum});
			advancePosition(1, pos);
			continue ;
		}

		else if (currentChar == ',')
		{
			_tokens.push_back({COMMA, ",", _lineNum, _colNum});
			advancePosition(1, pos);
			continue ;
		}

		//	--- Rule 4: Handle Strings
		else if (currentChar == '"' || currentChar == '\'')
		{
			std::string stringValue = consumeString(_input, pos, _lineNum, _colNum);
			_tokens.push_back({STRING, stringValue, _lineNum, _colNum});
			continue ;
		}

		//	--- Rule 5: Handle Words
		else if (isValidWordChar(currentChar))
		{
			std::string	wordValue = consumeWord(_input, pos);
			_tokens.push_back({WORD, wordValue, _lineNum, _colNum});
		}

		//	--- Rule 6: Handle Errors
		else
		{
			throw ConfigError(ErrorType::LEXER,
							"Unexpected character '" + std::string(1, currentChar) + "'",
							_lineNum, _colNum);
			pos++;	// Advance to avoid an infinite loop
		}
	}
	_tokens.push_back({END_OF_FILE, "", _lineNum, _colNum});
	return (this->_tokens);
}
