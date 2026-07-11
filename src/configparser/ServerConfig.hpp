#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include "ConfigTypes.hpp"

class			Directive;
class			ConfigFile;
struct			DirectiveDefinition;
extern const	std::map<std::string, DirectiveDefinition> NGINX_DIRECTIVE_SPECS;
class			Lexer;
class			Parser;
class			Validator;

class	ServerConfig
{
	private:
		std::string							_serverName;
		std::vector<ListenDirective>		_listenDirectives;
		unsigned long long					_maxBodySize;
		std::unordered_map<int, ErrorPage>	_errorPages;
		std::vector<Location>				_locations;
		int									_keepaliveTimeout;

	public:
		ServerConfig() = delete;
		ServerConfig(const std::string& serverName,
					const std::vector<ListenDirective>	listenDirectives,
					size_t maxBodySize,
					const std::unordered_map<int, ErrorPage>& errorPages,
					const std::vector<Location>& locations,
					int keepaliveTimeout);
		~ServerConfig() = default;

		const std::string&							getServerName() const { return this->_serverName; }
		const std::vector<ListenDirective>&			getListenDirectives() const { return this->_listenDirectives; }
		unsigned long long							getMaxBodySize() const { return this->_maxBodySize; }
		ErrorPage									getErrorPage(int code) const;
		const std::unordered_map<int, ErrorPage>&	getErrorPages() const { return this->_errorPages; }
		const std::vector<Location>&				getLocations() const { return this->_locations; }
		const Location&								getLocation(const std::string& name) const;
		int 										getKeepaliveTimeout() const { return this->_keepaliveTimeout; }
};
