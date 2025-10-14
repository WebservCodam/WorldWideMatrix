#include <../include/Lexer.hpp>

std::string	Lexer::consumeNumber(const std::string& input, size_t& pos)
{
	std::string	number;

	while (pos < input.length() && isdigit(input[pos]))
	{
		number += input[pos];
		pos++;
	}
	return (number);
}

bool	Lexer::isValidWordChar(char c)
{
	if (isspace(c) || c == '{' || c == '}' || c == ';' || c == '#') //There are more tokens now
		return (false);
	return (true);
}

std::string	Lexer::consumeWord(const std::string& input, size_t& pos)
{
	std::string	word;

	while (pos < input.length() && isValidWordChar(input[pos]))
	{
		word += input[pos];
		pos++;
	}
	return (word);
}

std::string	Lexer::consumeString(const std::string& input, size_t& pos)
{
	std::string	string;
	char		quote = input[pos++];	// Store and skip opening quote

	while (pos < input.length() && input[pos] != quote)
		string += input[pos++];

	if (pos >= input.length())
		throw std::runtime_error("Unterminated string literal");
	
	pos++;	// Skip closing quote
	return (string);
}


std::vector<Token>	Lexer::tokenize(const std::string& input)
{
	std::vector<Token>	tokens;			// These can be in the object
	size_t				line_num = 1;	//	"
	size_t				col_num = 1;	//	"
	size_t				pos = 0;		// This can be indeed a function's variable. 

	while (pos < input.length())
	{
		char	current_char = input[pos];	// Should it be reference or simply a new char?

		//	--- Rule 1: Handle Whitespace
		if (isspace(current_char))
		{
			if (current_char == '\n')
			{
				line_num++;
				col_num = 1;
			}
			else
				col_num++;
			pos++; // Consume the whitespace and continue to the next loop iteration.
			continue;
		}

		//	--- Rule 2: Handle Comments
		else if (current_char == '#')
		{
			while (pos < input.length() && input[pos] != '\n')
				pos++;
			continue ;
		}

		//	--- Rule 3: Handle Single-Character Tokens
		else if (current_char == '{')
		{
			tokens.push_back({LBRACE, "{", line_num, col_num}); // Replace with addToken(LBRACE, "{");
			pos++;		// Replace with advancePosition(1)
			col_num++;	// Replace with advancePosition (just one, the one from the previous comment).
			continue ;
		}

		else if (current_char == '}')
		{
			tokens.push_back({RBRACE, "}", line_num, col_num});
			pos++;
			col_num++;
			continue ;
		}

		else if (current_char == ';')
		{
			tokens.push_back({SEMICOLON, ";", line_num, col_num});
			pos++;
			col_num++;
			continue ;
		}

		else if (current_char == '=')
		{
			tokens.push_back({EQUALS, "=", line_num, col_num});
			pos++;
			col_num++;
			continue ;
		}

		// else if (current_char == '@')
		// {
		// 	tokens.push_back({AT, "@", line_num, col_num});
		// 	pos++;
		// 	col_num++;
		// 	continue ;
		// }

		else if (current_char == '[')
		{
			tokens.push_back({LBRACKET, "[", line_num, col_num});
			pos++;
			col_num++;
			continue ;
		}

		else if (current_char == ']')
		{
			tokens.push_back({RBRACKET, "]", line_num, col_num});
			pos++;
			col_num++;
			continue ;
		}

		else if (current_char == ',')
		{
			tokens.push_back({COMMA, ",", line_num, col_num});
			pos++;
			col_num++;
			continue ;
		}

		// else if (current_char == ':')
		// {
		// 	tokens.push_back({COLON, ":", line_num, col_num});
		// 	pos++;
		// 	col_num++;
		// 	continue ;
		// }

		else if (current_char == '"' || current_char == '\'')
		{
			std::string stringValue = consumeString(input, pos);
			tokens.push_back({STRING, stringValue, line_num, col_num});
			col_num += stringValue.length();
			continue ;
		}

		//	--- Rule 4: Handle Numbers
		else if (isdigit(current_char))
		{
			std::string	numberValue = consumeNumber(input, pos);
			// addToken(NUMBER, numberValue);
			tokens.push_back({NUMBER, numberValue, line_num, col_num});
			col_num += numberValue.length();
			continue ;
		}

		//	--- Rule 5: Handle Words
		else if (isValidWordChar(current_char))
		{
			std::string	wordValue = consumeWord(input, pos);
			tokens.push_back({WORD, wordValue, line_num, col_num});
			col_num += wordValue.length();
		}

		//	Rule 6: Handle Errors
		else
		{
			throw std::runtime_error("Unexpected character '" 
									+ std::string(1, current_char)
									+ "' at line " + std::to_string(line_num)
									+ ", column " + std::to_string(col_num));
			pos++;	// Just advance to avoid an infinite loop
		}
	}
	tokens.push_back({END_OF_FILE, "", line_num, col_num});
	return (tokens);
}
