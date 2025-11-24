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

	public:
		ConfigFile() = delete;
		ConfigFile(std::vector<std::unique_ptr<Directive>> directives);
		~ConfigFile() = default;

		const std::vector<std::unique_ptr<Directive>>&	getDirectives() const;

		// Query methods
		const Directive*				findDirective(const std::string& name) const;
		std::vector<const Directive*>	findAllDirectives(const std::string& name) const;
};

class	Location
{
	private:
		std::string					_path;
		std::string					_root;
		std::string					_index;
		bool						_autoindex;
		// std::vector<std::string>	_allowedMethods;
		bool						_getMethod;
		bool						_postMethod;
		bool						_deleteMethod;

	public:
		Location() = delete;
		Location(const std::string& path, const std::string& root = "", const std::string& index = "", bool autoindex = false, bool getMethod = true, bool postMethod = false, bool deleteMethod = false);
		~Location() = default;


};

class	Server
{
	private:
		std::string								_serverName;
		std::map<std::string, std::string>		_addressesAndPorts;	//Defaults to 0.0.0.0:80
		size_t									_maxBodySize;
		std::map<std::vector<int>, std::string>	_error;	//	The idea is that different error codes can return the same error. But this might overcomplicate things.
		std::vector<Location>					_location;

	public:
		Server() = default;
		Server(const std::string& serverName, const std::map<std::string, std::string>& addressesAndPorts = {{"0.0.0.0", "80"}}, size_t maxBodySize = 1048576, const std::map<std::vector<int>, std::string>& error = {}, const std::vector<Location>& location = {});
		~Server() = default;

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
