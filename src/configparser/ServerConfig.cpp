#include "Configuration.hpp"

//	----- SERVER -----

ServerConfig::ServerConfig(const std::string& serverName,
				const std::vector<ListenDirective>	listenDirectives,
				size_t maxBodySize,
				const std::unordered_map<int, ErrorPage>& errorPages,
				const std::vector<Location>& locations,
				int keepalive_timeout)
	: _serverName(serverName), _listenDirectives(listenDirectives), _maxBodySize(maxBodySize), _errorPages(errorPages), _locations(locations), _keepalive_timeout(keepalive_timeout)
{
}

ErrorPage	ServerConfig::getErrorPage(int code) const
{
	ErrorPage	errorPage;

	try
	{
		errorPage = this->_errorPages.at(code);
	} catch (const std::out_of_range&)
	{
		errorPage = this->_errorPages.at(code / 100);
	}
	return (errorPage);
}

const Location&	ServerConfig::getLocation(const std::string& name) const
{
	std::string	trimmedName = trimPathName(name);

	for (const Location& location : _locations)
	{
		if (trimmedName == location.name)
			return (location);
	}
	throw ConfigError::semantics("Couldn't find a location with the given name: " + name, nullptr);
}
