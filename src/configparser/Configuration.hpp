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
#include <sys/stat.h>
#include <unistd.h>

# define DEFAULT_KEEP_ALIVE_TIMEOUT 30
# define DEFAULT_MAX_BODY_SIZE 2000000
# define DEFAULT_ROOT_PATH "/www/"
# define DEFAULT_ERROR_PAGES_PATH "/error_pages/"
# define DEFAULT_40x_ERROR_PAGE "40x.html"
# define DEFAULT_50x_ERROR_PAGE "50x.html"

enum	TokenType
{
	WORD,
	LBRACE,		// {
	RBRACE,		// }
	SEMICOLON,	// ;
	COMMA,		// ,
	STRING,		// "quoted string" or 'single quoted'
	END_OF_FILE	// Special token for the end of the input
};

enum class ErrorType
{
	INITIALIZATION,
	LEXER,
	PARSER,
	VALIDATOR,
	SEMANTICS
};

class			ServerConfig;
class			Directive;
class			ConfigFile;
struct			DirectiveDefinition;
extern const	std::map<std::string, DirectiveDefinition> NGINX_DIRECTIVE_SPECS;
class			Location;
class			Lexer;
class			Parser;
class			Validator;
struct			ListenDirective;
struct			ErrorPage;
struct			ReturnPage;

// ===============  TOKEN  ===============

struct	Token
{
	TokenType	type;
	std::string	value;

	size_t		line;
	size_t		column;
};

// ===============  DIRECTIVE DEFINITION  ===============

struct	DirectiveDefinition
{
	std::string								name;
	bool									isBlock;
	size_t									minArgs;
	size_t									maxArgs;
	std::set<std::string>					validContexts;
	std::set<std::string>					requiredChildren;

	void (*validateArgs)(Directive*);
};

// ===============  CONFIG ERROR  ===============

class ConfigError : public std::runtime_error
{
	private:
		ErrorType	_type;
		size_t		_line;
		size_t		_column;
		std::string	_context;

		static std::string	buildMessage(ErrorType type, const std::string& message, size_t line, size_t column, const std::string& context);

	public:

		ConfigError(ErrorType type, const std::string& message);
		ConfigError(ErrorType type, const std::string& message, size_t line, size_t column, const std::string& context = "");
		ConfigError(ErrorType type, const std::string& message, const Directive* directive);
		~ConfigError() = default;

		static std::string	buildMessage(ErrorType type, const std::string& message);
		static ConfigError	initialization(const std::string& message);
		static ConfigError	lexing(const std::string& message, size_t line, size_t column);
		static ConfigError	parsing(const std::string& message, size_t line, size_t column);
		static ConfigError	validation(const std::string& message, const Directive* directive);
		static ConfigError	semantics(const std::string& message, const Directive* directive);

		const char*			what() const noexcept override;
};

// ===============  LEXER  ===============

class	Lexer
{
	private:
		std::string			_input;
		std::vector<Token>	_tokens;
		size_t				_line_num = 1;
		size_t				_col_num = 1;

		void		advancePosition(int len, size_t& pos);
		bool		isValidWordChar(char c);
		std::string	consumeWord(const std::string& input, size_t& pos);
		std::string	consumeString(const std::string& input, size_t& pos, size_t line, size_t col);

	public:
		Lexer() = delete; // Could default to a config file.
		Lexer(const std::string& input) { this->_input = input; };
		~Lexer() = default;

		std::vector<Token>	tokenize();

		// std::vector<Token>	getTokens();
};

// ===============  PARSER  ===============

class	Parser
{
	private:
		std::string					_input;
		std::vector<Token>			_tokens;
		size_t						_currentIndex;
		std::unique_ptr<ConfigFile>	_configFile;

		const Token&	currentToken() const;
		void			advance();
		const Token&	peekToken(size_t offset = 1) const;	// Default = 1
		bool			isAtEnd();
		void			expectToken(TokenType type, const std::string& errorMessage);

		std::unique_ptr<Directive>			initializeDirective();
		std::unique_ptr<Directive>			parseDirective();
		std::vector<std::string>			parseParameters();

		void	checkDefaultErrorPages(std::vector<std::unique_ptr<Directive>>& directives);
		void	validateSemantics();

	public:
		Parser() = delete;
		Parser(const std::string& input);
		~Parser() = default;

		std::unique_ptr<ConfigFile>	parse();
};

// ===============  SERVER CONFIG  ===============

struct ListenDirective
{
	std::string	address;
	std::string	port;
    
	ListenDirective(const std::string& addr, const std::string& p) : address(addr), port(p) {}
};

struct	ErrorPage
{
	int			errorCode;
	bool		isRedirect = false;
	int			redirectCode = -1;
	std::string	URI;

	ErrorPage() = default;
	ErrorPage(int code, const std::string& uri) : errorCode(code), isRedirect(false), redirectCode(-1), URI(uri) {}
};

struct	ReturnPage
{
	int			code = -1;
	bool		isURI = false;
	std::string	page = "";	// Which can store a path to a page if it is URI or it can simply store the whole page here.
};

struct	Location
{
	std::string					name = "";			// This represents the location we're trying to access.
	std::string					dirPath = "";
	std::string					indexPath = "";		// This is the full path that goes to the index. root + location + (index?).
	ReturnPage					returnPage = ReturnPage();
	bool						autoindex = false;
	bool						getMethod = false;
	bool						postMethod = false;
	bool						deleteMethod = false;
};

// =============== --- DIRECTIVE --- ===============

class	Directive
{
	private:
		size_t _line;
		size_t _column;

		std::string									_name;
		std::string									_context;
		std::vector<std::string>					_parameters;
		Directive*									_parent = nullptr;
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
		size_t							getLine() const { return this->_line; }
		size_t							getColumn() const { return this->_column; }
		const std::string&				getName() const { return this->_name; }
		const std::string&				getContext() const { return this->_context; }
		const std::string&				getParameter(size_t i) const;
		const std::vector<std::string>&	getParameters() const { return this->_parameters; }
		Directive*						getParent() { return this->_parent; }
		Directive*						getChild(size_t i);
		Directive*						getChild(const std::string& name);
		std::vector<Directive*>			getChildren();

		// Setters
		void	setLine(size_t line) { this->_line = line; }
		void	setColumn(size_t column) { this->_column = column; }
		void	setName(const std::string& name) { this->_name = name; }
		void	setContext(const std::string& context) { this->_context = context; }
		void	setParameter(int index, const std::string& new_parameter);
		void	setParameters(const std::vector<std::string>& parameters) { this->_parameters = parameters; }
		void	addChild(std::unique_ptr<Directive> child) { this->_children.push_back(std::move(child)); }
		void	setParent(Directive* parent) { this->_parent = parent; }
};

// =============== --- CONFIG FILE --- ===============

class	ConfigFile
{
	private:
		std::vector<std::unique_ptr<Directive>>	_directives;
		std::vector<ServerConfig>				_servers;
		
	public:
		ConfigFile() = delete;
		ConfigFile(std::vector<std::unique_ptr<Directive>> directives);
		~ConfigFile() = default;

		std::vector<std::unique_ptr<Directive>>&	getDirectives(); // Non constant so the validation can modify the directives.
		std::vector<ServerConfig>	getServers() const { return (this->_servers); }
		ServerConfig				getServer(const std::string& serverName) const;

		std::vector<ServerConfig>	createServers();

	private:
		// Helper functions for processing server directives
		std::string							processServerName(const Directive* directive);
		void								processListen(const Directive* directive, std::vector<ListenDirective>& listenDirectives);
		unsigned long long					processClientMaxBodySize(const Directive* directive);
		Location							processLocation(Directive* directive);
		int									processKeepaliveTimeout(Directive* directive);
		void								processErrorPages(Directive* directive, std::unordered_map<int, ErrorPage>&);
		ReturnPage							processReturnPage(const Directive* directive);

	public:
		// Query methods
		const Directive*		findDirective(const std::string& name) const;
		std::vector<Directive*>	findAllDirectives(const std::string& name) const;
};

class	ServerConfig
{
	private:
		std::string							_serverName;
		std::vector<ListenDirective>		_listenDirectives;
		unsigned long long					_maxBodySize;
		std::unordered_map<int, ErrorPage>	_errorPages;
		std::vector<Location>				_locations;
		int									_keepalive_timeout;

	public:
		ServerConfig() = delete;
		ServerConfig(const std::string& serverName,
					const std::vector<ListenDirective>	listenDirectives,
					size_t maxBodySize,
					const std::unordered_map<int, ErrorPage>& errorPages,
					const std::vector<Location>& locations,
					int keepalive_timeout);
		~ServerConfig() = default;

		// For the return directive -> if the page string is empty, and the code is an error page, then serve the error page for that code.
		// If there is no error page for that code, serve the default error page.

		// Create function that retrieves an error page from the code given. If the page is not specified, then return the default error page.

		const std::string&							getServerName() const { return this->_serverName; }
		const std::vector<ListenDirective>&			getListenDirectives() const { return this->_listenDirectives; }
		unsigned long long							getMaxBodySize() const { return this->_maxBodySize; }
		ErrorPage									getErrorPage(int code) const;
		const std::unordered_map<int, ErrorPage>&	getErrorPages() const { return this->_errorPages; }
		const std::vector<Location>&				getLocations() const { return this->_locations; }
		int 										getKeepaliveTimeout() const { return this->_keepalive_timeout; }
};

// =============== --- DIRECTIVE SPECS --- ===============

void	validateDirective(Directive* node);
void	validateBlockDirective(Directive* node);
void	validateContext(Directive* node);
void	validateRequiredChildren(Directive* node);

// void	validateHttpDirective(Directive* node);
void	validateServerDirective(Directive* node);
void	validateLocationDirective(Directive* node);
void	validateListenDirective(Directive* node);
void	validateRootDirective(Directive* node);
void	validateIndexDirective(Directive* node);
void	validateAutoIndexDirective(Directive* node);
void	validateErrorPageDirective(Directive* node);
// void	validateFastcgiPassDirective(Directive* node);
// void	validateFastcgiParamDirective(Directive* node);
// void	validateFastcgiIndexDirective(Directive* node);
void	validateReturnDirective(Directive* node);
void	validateMethodsDirective(Directive* node);
void	validateClientMaxBodySizeDirective(Directive* node);
// void	validateAllowOrDenyDirective(Directive* node);	// Can be used to block certain IP Addresses from accessing a page.
void	validateKeepaliveTimeoutDirective(Directive* node);
// void	validateUploadPathDirective(Directive* node);

// =============== --- Utilities --- ===============

Directive*							getServerDirective(Directive *node);
bool								getAutoindex(Directive *node);
std::string							getRoot(Directive *node);
std::pair<std::string, std::string>	parseAddressAndPort(const std::string& address);
bool 								isByte(std::string &number);
bool								validateAddress(const std::string& address);
bool								validatePort(const std::string& port);

std::string	joinPath(const std::string& a, const std::string& b);
void		checkPath(const std::string& path, ErrorType errorType, const std::string& msg1, const std::string& msg2);