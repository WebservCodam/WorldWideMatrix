#include "Configuration.hpp"

ConfigFile::ConfigFile(std::vector<std::unique_ptr<Directive>> directives) : _directives(std::move(directives)) {}

const Directive*	ConfigFile::findDirective(const std::string& name) const
{
	if (this->_directives.size() == 0)
		throw ConfigError::custom(ErrorType::PARSER, "There are no directives in the configuration file!");
	for (const std::unique_ptr<Directive>& directive : this->_directives)
	{
		if (directive && directive->getName() == name)
			return (directive.get()); // Extracts raw pointer from unique_ptr
	}
	return (nullptr);
}

std::vector<Directive*>	ConfigFile::findAllDirectives(const std::string& name) const
{
	std::vector<Directive*>	result;

	if (this->_directives.size() == 0)
		throw ConfigError::custom(ErrorType::PARSER, "There are no directives in the configuration file!");
	for (size_t i = 0; i < this->_directives.size(); ++i)
	{
		Directive* directive = this->_directives[i].get();

		if (!directive)
			continue ;

		if (directive->getName() == name)
			result.push_back(directive);
	}
	return (result);
}

ServerConfig	ConfigFile::getServer(const std::string& serverName) const
{
	for (const ServerConfig& server : _servers)
	{
		if (server.getServerName() == serverName)
			return server;
	}

	throw ConfigError::custom(
		ErrorType::SEMANTICS,
		"ServerConfig with name '" + serverName + "' not found");
}

std::vector<ServerConfig>	ConfigFile::createServers()
{
	std::vector<Directive*> serverDirectives = this->findAllDirectives("server");

	for (Directive* serverDirective : serverDirectives)
	{
		if (serverDirective->getName() != "server")
			continue ;

		std::string							serverName = "unnamed_server";
		std::vector<ListenDirective>		listenDirectives;
		unsigned long long					maxBodySize = DEFAULT_MAX_BODY_SIZE;
		std::unordered_map<int, ErrorPage>	errorPages;
		std::vector<Location>				locations;
		int									keepaliveTimeout = DEFAULT_KEEP_ALIVE_TIMEOUT;

		std::vector<Directive*> serverChildren = serverDirective->getChildren();

		for (Directive* directive : serverChildren)
		{
			if (!directive)
				continue;

			if (directive->getName() == "server_name")
				serverName = processServerName(directive);
			else if (directive->getName() == "listen")
				processListen(directive, listenDirectives);
			else if (directive->getName() == "client_max_body_size")
				maxBodySize = processClientMaxBodySize(directive);
			else if (directive->getName() == "error_page")
				processErrorPages(directive, errorPages);
			else if (directive->getName() == "location")
				locations.push_back(processLocation(directive));
			else if (directive->getName() == "keepalive_timeout")
				keepaliveTimeout = processKeepaliveTimeout(directive);
		}

		ServerConfig server(serverName, listenDirectives, maxBodySize, errorPages, locations, keepaliveTimeout);
		this->_servers.push_back(server);
	}

	return (this->_servers);
}
