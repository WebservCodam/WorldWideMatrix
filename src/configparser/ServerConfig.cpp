#include "ServerConfig.hpp"
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

// Returns the error page configured for `code`, or throws std::out_of_range
// if none was set (the caller then serves a generated default page).
ErrorPage	ServerConfig::getErrorPage(int code) const
{
	return (this->_errorPages.at(code));
}

const Location&	ServerConfig::getLocation(const std::string& uri) const
{
	const Location*	best = nullptr;
	std::size_t		bestLen = 0;
	
	std::cout << "DEBUG getLocation" << std::endl;
	std::cout << "DEBUG - uri is: " + uri << std::endl;

	for (const Location& location : _locations)
	{
		// Rebuild the location path with a leading slash: "images" -> "/images",
		// root stays "/". This is what we test the request URI against.
		std::string	prefixLocation = (location.name == "/") ? "/" : "/" + location.name;
		
		std::cout << "DEBUG - prefixLocation is: " + prefixLocation << std::endl;

		// The URI must start with the location path...
		if (uri.compare(0, prefixLocation.size(), prefixLocation) != 0)
			continue;
		// ...and line up on a whole path segment, so "/images" does not match "/imagesXYZ".
		bool	matchesWholeSegment = (prefixLocation == "/")
			|| uri.size() == prefixLocation.size()
			|| uri[prefixLocation.size()] == '/';
		// Keep the longest match ('>' so root, length 1, is only a fallback).
		if (matchesWholeSegment && prefixLocation.size() > bestLen)
		{
			best = &location;
			bestLen = prefixLocation.size();
		}
	}
	if (best == nullptr)
		throw ConfigError::semantics("Couldn't find a location matching the URI: " + uri, nullptr);
	std::cout << "DEBUG - best location is: " + best->name << std::endl;
	return (*best);
}
