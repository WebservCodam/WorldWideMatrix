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

class	ConfigFile
{
	private:
		std::vector<std::unique_ptr<Directive>>	_directives;

	public:
		ConfigFile() = delete;
		ConfigFile(std::vector<std::unique_ptr<Directive>> directives);
		~ConfigFile() = default;

		const std::vector<std::unique_ptr<Directive>>&	getDirectives() const;

		// Query methods
		const Directive*				findDirective(const std::string& name) const;
		std::vector<const Directive*>	findAllDirectives(const std::string& name) const;
};

class	Directive
{
	private:
		size_t _line;
		size_t _column;

		std::string									_name;
		std::string									_context;
		std::vector<std::string>					_parameters;
		std::vector<std::unique_ptr<Directive>>		_children;

	public:
		Directive() = default;
		Directive(
			size_t line, 
			size_t column, 
			const std::string& name, 
			const std::string& context, 
			std::vector<std::string> parameters, 				// by value, so we can use std::move
			std::vector<std::unique_ptr<Directive>> children);	// by value, so we can use std::move
		~Directive() = default;

		// Getters
		size_t							getLine() const;
		size_t							getColumn() const;
		const std::string&				getName() const;
		const std::string&				getContext() const;
		const std::vector<std::string>&	getParameters() const;
		std::vector<const Directive*>	getChildren() const;

		// Setters (Move definitions to configuration.cpp?)
		void setLine(size_t line) { _line = line; }
		void setColumn(size_t column) { _column = column; }
		void setName(const std::string& name) { _name = name; }
		void setContext(const std::string& context) { _context = context; }
		void setParameters(const std::vector<std::string>& parameters) { _parameters = parameters; }

		void addChild(std::unique_ptr<Directive> child) {
			_children.push_back(std::move(child));
		}
};

struct DirectiveDefinition
{
	std::string								name;
	bool									isBlock;
	size_t									minArgs;
	size_t									maxArgs;
	std::set<std::string>					validContexts;
	std::set<std::string>					requiredChildren;

	bool (*validateArgs)(const Directive*);
};

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
