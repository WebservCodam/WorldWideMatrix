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

// Put all of this below inside the class in the header file.

const std::string&	ServerConfig::getServerName() const
{
	return (this->_serverName);
}

const std::vector<ListenDirective>&	ServerConfig::getListenDirectives() const
{
	return (this->_listenDirectives);
}

unsigned long long	ServerConfig::getMaxBodySize() const
{
	return (this->_maxBodySize);
}

const std::unordered_map<int, ErrorPage>&	ServerConfig::getErrorPages() const
{
	return (this->_errorPages);
}

const std::vector<Location>&	ServerConfig::getLocations() const
{
	return (this->_locations);
}

int	ServerConfig::getKeepaliveTimeout() const
{
	return (this->_keepalive_timeout);
}
