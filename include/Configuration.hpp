#pragma once

#include <iostream>
#include <string>
#include <vector>

enum TokenType
{
	STRING,
	NUMBER,
	LBRACE,		// {
	RBRACE,		// }
	SEMICOLON,	// ;
	COMMENT,	// # ...
	END_OF_FILE	// Special token for the end of the input
};

struct Token
{
	TokenType	type;
	std::string	value;

	size_t		line;
	size_t		column;
};

