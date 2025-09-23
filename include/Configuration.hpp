#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>

enum TokenType
{
	WORD,
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

// AST Node Types
struct ASTNode
{
	size_t line;
	size_t column;

	virtual ~ASTNode() = default;
};

struct SimpleDirective : public ASTNode
{
	std::string					name;
	std::vector<std::string>	parameters;
};

struct BlockDirective : public ASTNode
{
	std::string									name;
	std::vector<std::string>					parameters;
	std::vector<std::unique_ptr<ASTNode>>		children;
};

struct ConfigFile : public ASTNode
{
	std::vector<std::unique_ptr<ASTNode>>	directives;
};

