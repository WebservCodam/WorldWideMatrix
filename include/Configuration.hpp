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
#include <Server.hpp>

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
		const std::string&				getParameter(size_t i) const;
		const std::vector<std::string>&	getParameters() const;
		const Directive*				getChild(size_t i) const;
		std::vector<const Directive*>	getChildren() const;

		// Setters
		void setLine(size_t line);
		void setColumn(size_t column);
		void setName(const std::string& name);
		void setContext(const std::string& context);
		void setParameters(const std::vector<std::string>& parameters);
		void addChild(std::unique_ptr<Directive> child);
};

class	ConfigFile
{
	private:
		std::vector<std::unique_ptr<Directive>>	_directives;
		std::vector<Server>						_servers;

	public:
		ConfigFile() = delete;
		ConfigFile(std::vector<std::unique_ptr<Directive>> directives);
		~ConfigFile() = default;

		const std::vector<std::unique_ptr<Directive>>&	getDirectives() const;
		const std::vector<Server>&						getServers() const;
		const Server&									getServer(const std::string& serverName);

		void	createServers();

		// Query methods
		const Directive*				findDirective(const std::string& name) const;
		std::vector<const Directive*>	findAllDirectives(const std::string& name) const;
};

enum TokenType
{
	WORD,
	NUMBER,
	LBRACE,		// {
	RBRACE,		// }
	SEMICOLON,	// ;
	EQUALS,		// =
	STRING,		// "quoted string" or 'single quoted'
	LBRACKET,	// [
	RBRACKET,	// ]
	COMMA,		// ,
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
