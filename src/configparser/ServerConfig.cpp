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

ErrorPage	ServerConfig::getErrorPage() const
{
	
}
