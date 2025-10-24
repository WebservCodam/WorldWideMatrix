#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <functional>
#include <set>
#include <map>
#include <unordered_map>

// #include "DirectiveSpecs.hpp"

enum TokenType
{
	WORD,
	NUMBER,
	LBRACE,		// {
	RBRACE,		// }
	SEMICOLON,	// ;
	EQUALS,		// =
	// AT,		// @	This token is actually unnecessary and would only make things more complex
	STRING,		// "quoted string" or 'single quoted'
	LBRACKET,	// [
	RBRACKET,	// ]
	COMMA,		// ,
	// COLON,	// :	Would interefere with links, so wherever it was needed (interface:port) should be parsed separately
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

struct DirectiveDefinition
{
	std::string				name;
	bool					isBlock;
	size_t					minArgs;
	size_t					maxArgs;
	std::set<std::string>	validContexts;
	// std::function<bool(const std::vector<std::string>&)> validateArgs; //It's a function pointer.
};

// AST Node Types
struct Directive
{
	size_t line;
	size_t column;

	std::string					name;
	std::vector<std::string>					parameters;
	std::vector<std::unique_ptr<Directive>>		children;

	virtual ~Directive() = default;	// So that the smart pointer (unique_ptr) can destroy the object.
};

struct ConfigFile : public Directive
{
	std::vector<std::unique_ptr<Directive>>	directives;
};

//	---------- PRINT UTILITIES ---------- 

inline const char*		tokenTypeToString(TokenType type);
inline std::ostream&	operator<<(std::ostream& os, const Token& token);
void					printTokensList(const std::vector<Token>& tokenList);
void					printIndent(int indent, const std::string& prefix = "");
void					printSimpleDirective(const Directive* directive, int indent, const std::string& prefix);
void					printBlockDirective(const Directive* directive, int indent, const std::string& prefix);
void					printASTNode(const Directive* node, int indent = 0, const std::string& prefix = "");
void					printAST(const std::vector<std::unique_ptr<Directive>>& config);

//