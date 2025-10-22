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
	// AT,			// @	This token is actually unnecessary and would only make things more complex
	STRING,		// "quoted string" or 'single quoted'
	LBRACKET,	// [
	RBRACKET,	// ]
	COMMA,		// ,
	// COLON,		// :	Would interefere with links, so wherever it was needed (interface:port) should be parsed separately
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
struct ASTNode
{
	size_t line;
	size_t column;

	virtual ~ASTNode() = default;	// So that the smart pointer (unique_ptr) can destroy the object.
};

// name value ;
// name { more directives }

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

//	---------- PRINT UTILITIES ---------- 

inline const char*		tokenTypeToString(TokenType type);
inline std::ostream&	operator<<(std::ostream& os, const Token& token);
void					printTokensList(const std::vector<Token>& tokenList);
void					printIndent(int indent, const std::string& prefix = "");
void					printSimpleDirective(const SimpleDirective* directive, int indent, const std::string& prefix);
void					printBlockDirective(const BlockDirective* directive, int indent, const std::string& prefix);
void					printASTNode(const ASTNode* node, int indent = 0, const std::string& prefix = "");
void					printAST(const std::unique_ptr<ConfigFile>& config);

//