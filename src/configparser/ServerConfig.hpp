#pragma once

#include "Configuration.hpp"

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
		const Location&								getLocation(const std::string& name) const;
		int 										getKeepaliveTimeout() const { return this->_keepalive_timeout; }
};
