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

enum	TokenType
{
	WORD,
	NUMBER,
	LBRACE,		// {
	RBRACE,		// }
	SEMICOLON,	// ;
	COMMA,		// ,
	STRING,		// "quoted string" or 'single quoted'
	END_OF_FILE	// Special token for the end of the input
};

enum class ErrorType
{
	LEXER,
	PARSER,
	VALIDATOR
};



class	ServerConfig;
class	Directive;
class	ConfigFile;
struct	DirectiveDefinition;
extern const std::map<std::string, DirectiveDefinition> NGINX_DIRECTIVE_SPECS;
class	Location;
class	Lexer;
class	Parser;
class	Validator;
struct	ListenDirective;

// --- TOKEN ---

struct	Token
{
	TokenType	type;
	std::string	value;

	size_t		line;
	size_t		column;
};

// --- DIRECTIVE DEFINITION ---

struct	DirectiveDefinition
{
	std::string								name;
	bool									isBlock;
	size_t									minArgs;
	size_t									maxArgs;
	std::set<std::string>					validContexts;
	std::set<std::string>					requiredChildren;

	bool (*validateArgs)(const Directive*);
};

// --- DIRECTIVE ---

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

// --- CONFIG FILE ---

class	ConfigFile
{
	private:
		std::vector<std::unique_ptr<Directive>>	_directives;
		std::vector<ServerConfig>				_servers;
		
	public:
		ConfigFile() = delete;
		ConfigFile(std::vector<std::unique_ptr<Directive>> directives);
		~ConfigFile() = default;

		const std::vector<std::unique_ptr<Directive>>&	getDirectives() const;
		const std::vector<ServerConfig>&				getServers() const;
		const ServerConfig&								getServer(const std::string& serverName);

		std::vector<ServerConfig>	createServers();

	private:
		// Helper functions for processing server directives
		void		processServerName(const Directive* directive, std::string& serverName);
		void		processListen(const Directive* directive, std::vector<ListenDirective>& listenDirectives);
		void		processClientMaxBodySize(const Directive* directive, size_t& maxBodySize);
		void		processErrorPage(const Directive* directive, std::map<int, std::string>& errors);
		Location	processLocation(const Directive* directive);

	public:
		// Query methods
		const Directive*				findDirective(const std::string& name) const;
		std::vector<const Directive*>	findAllDirectives(const std::string& name) const;
};

// --- DIRECTIVE SPECS ---

bool	validateHttpDirective(const Directive* node);
bool	validateServerDirective(const Directive* node);
bool	validateLocationDirective(const Directive* node);
bool	validateListenDirective(const Directive* node);
bool	validateRootDirective(const Directive* node);
bool	validateIndexDirective(const Directive* node);
bool	validateAutoIndexDirective(const Directive* node);
bool	validateErrorPageDirective(const Directive* node);
// bool	validateFastcgiPassDirective(const Directive* node);
// bool	validateFastcgiParamDirective(const Directive* node);
// bool	validateFastcgiIndexDirective(const Directive* node);
bool	validateReturnDirective(const Directive* node);
bool	validateAllowMethodsDirective(const Directive* node);
bool	validateClientMaxBodySizeDirective(const Directive* node);
bool	validateAllowOrDenyDirective(const Directive* node);	// Can be used to block certain IP Addresses from accessing a page.

std::pair<std::string, std::string>	parseAddressAndPort(const std::string& address);
bool 								isByte(std::string &number);
bool								validateAddress(const std::string& address);
bool								validatePort(const std::string& port);
bool								validateContext(const Directive* node);
bool								validateRequiredChildren(const Directive* node);

// --- CONFIG ERROR ---

class ConfigError : public std::runtime_error
{
	private:
		ErrorType	_type;
		size_t		_line;
		size_t		_column;
		std::string	_context;

		static std::string	buildMessage(ErrorType type, const std::string& message,
									size_t line, size_t column, const std::string& context);

	public:
		ConfigError(ErrorType type, const std::string& message,
					size_t line, size_t column,
					const std::string& context = "");

		ConfigError(ErrorType type, const std::string& message,
					const Directive* directive);

		static ConfigError	parsing(const std::string& message, size_t line, size_t column);
		static ConfigError	validation(const std::string& message, const Directive* directive);

		ErrorType			getType() const { return _type; }
		size_t				getLine() const { return _line; }
		size_t				getColumn() const { return _column; }
		const std::string&	getContext() const { return _context; }

		const char*			what() const noexcept override;
};

// --- LEXER ---

class	Lexer
{
	private:
		std::string			_input;
		std::vector<Token>	_tokens;
		size_t				_line_num = 1;
		size_t				_col_num = 1;

		std::string	consumeNumber(const std::string& input, size_t& pos);
		bool		isValidWordChar(char c);
		std::string	consumeWord(const std::string& input, size_t& pos);
		std::string	consumeString(const std::string& input, size_t& pos, size_t line, size_t col);

	public:
		Lexer() = delete; // Can default to a config file.
		Lexer(const std::string& input);
		~Lexer();

		std::vector<Token>	tokenize();

		// std::vector<Token>	getTokens();
};

// --- PARSER ---

class	Parser
{
	private:
		std::string			_input;
		std::vector<Token>	_tokens;
		size_t				_currentIndex;

		const Token&	currentToken() const;
		void			advance();
		const Token&	peekToken(size_t offset = 1) const;	// Default = 1
		bool			isAtEnd();
		void			expectToken(TokenType type, const std::string& errorMessage);

		std::unique_ptr<Directive>			initializeDirective();
		std::unique_ptr<Directive>			parseDirective();
		std::unique_ptr<Directive>			parseSimpleDirective();
		std::unique_ptr<Directive>			parseBlockDirective();
		std::vector<std::string>			parseParameters();

	public:
		Parser() = delete;
		Parser(const std::string& input);
		~Parser() = default;

		std::unique_ptr<ConfigFile>	parse();
};

// --- VALIDATOR --- 

class	Validator
{
	private:
		const ConfigFile*							_ConfigFile;
		std::map<std::string, DirectiveDefinition>	_directiveSpecs;

	public:
		Validator() = delete;
		Validator(const ConfigFile* configFile);
		~Validator() = default;

		bool	validate();

	private:
		bool	validateDirective(const Directive* node);
};

// --- SERVER CONFIG ---

struct ListenDirective
{
	std::string	address;	// defaults to "0.0.0.0"
	std::string	port;		// defaults to 8080
    
	ListenDirective(const std::string& addr = "0.0.0.0", std::string p = "8080") : address(addr), port(p) {}
};

class	Location
{
	private:
		std::string					_path;
		std::string					_root;
		std::string					_index;
		bool						_autoindex;
		bool						_getMethod;
		bool						_postMethod;
		bool						_deleteMethod;

	public:
		Location() = delete;
		Location(const std::string& path, const std::string& root = "", const std::string& index = "", bool autoindex = false, bool getMethod = true, bool postMethod = false, bool deleteMethod = false);
		~Location() = default;

		const std::string&	getPath() const;
		const std::string&	getRoot() const;
		const std::string&	getIndex() const;
		bool				getAutoindex() const;
		bool				getGetMethod() const;
		bool				getPostMethod() const;
		bool				getDeleteMethod() const;
};

// --- SERVER CONFIG ---

class	ServerConfig
{
	private:
		std::string						_serverName;
		std::vector<ListenDirective>	_listenDirectives;
		size_t							_maxBodySize;
		std::map<int, std::string>		_errors;	//	The idea is that different error codes can return the same error. But this might overcomplicate things.
		std::vector<Location>			_locations;

	public:
		ServerConfig() = delete;
		ServerConfig(const std::string& serverName, const std::vector<ListenDirective>	listenDirectives, size_t maxBodySize, const std::map<int, std::string>& errors, const std::vector<Location>& locations);
		~ServerConfig() = default;

		const std::string&						getServerName() const;
		const std::vector<ListenDirective>&		getListenDirectives() const;	// Create function to get a port from an address and viceversa
		size_t									getMaxBodySize() const;
		const std::map<int, std::string>&		getErrors() const;
		const std::vector<Location>&			getLocations() const;
};
