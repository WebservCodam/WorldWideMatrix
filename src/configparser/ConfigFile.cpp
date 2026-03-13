#include "Configuration.hpp"

ConfigFile::ConfigFile(std::vector<std::unique_ptr<Directive>> directives) : _directives(std::move(directives)) {}

const Directive*	ConfigFile::findDirective(const std::string& name) const
{
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

std::vector<std::unique_ptr<Directive>>&	ConfigFile::getDirectives()
{
    return (_directives);
}

ServerConfig	ConfigFile::getServer(const std::string& serverName) const
{
	for (const ServerConfig& server : _servers)
	{
		if (server.getServerName() == serverName)
			return server;
	}

	throw ConfigError::buildMessage(
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
		int									keepalive_timeout = DEFAULT_KEEP_ALIVE_TIMEOUT;

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
				keepalive_timeout = processKeepaliveTimeout(directive);
		}

		ServerConfig server(serverName, listenDirectives, maxBodySize, errorPages, locations, keepalive_timeout);
		this->_servers.push_back(server);
	}

	return (this->_servers);
}

// ----- PROCESSING FUNCTIONS -----

/**
 * @return
 * If there's a server name directive, it returns the given server name.
 * Otherwise it returns 'unnamed_server'.
 */
std::string	ConfigFile::processServerName(const Directive* directive)
{
	if (directive)
		return (directive->getParameter(0));
	return ("unnamed_server");
}

/**
 * @brief
 * This function takes a vector of ListenDirective objects to store the resulting address & port pair.
 * It is possible for a listen directive to only have an address or a port.
 * 
 * @return
 * In the case where there's only an address, it returns it with a default port of '8080'.
 * In the case where there's only a port, it returns it with a default address of '127.0.0.1'.
 * In the case where both are provided it pushes that ListenDirective into the vector.
 * In any other case it throws an error.
 */
void	ConfigFile::processListen(const Directive* directive, std::vector<ListenDirective>& listenDirectives)
{
	if (directive)
	{
		std::string	listenParam = directive->getParameter(0);
		size_t		colonPos = listenParam.find(':');

		if (colonPos != std::string::npos)
		{
			std::string address = listenParam.substr(0, colonPos);
			std::string port = listenParam.substr(colonPos + 1);
			listenDirectives.push_back(ListenDirective(address, port));
		}
		else
		{
			if (validateAddress(listenParam))
				listenDirectives.push_back(ListenDirective(listenParam, "8080"));
			else if (validatePort(listenParam))
				listenDirectives.push_back(ListenDirective("127.0.0.1", listenParam));
			else
				throw ConfigError::semantics("Invalid address & port", directive);
		}
	}
}

/**
 * @brief
 * During validation the valid parameters that had letters have been changed to the number equivalent.
 * @return
 * This function simply returns the conversion of that number stored in a string into an unsigned long long.
 */
unsigned long long	ConfigFile::processClientMaxBodySize(const Directive* directive)
{
	if (directive)
		return (std::stoull(directive->getParameter(0)));
	return (DEFAULT_MAX_BODY_SIZE);
}

/**
 * @brief
 * 
 */
Location	ConfigFile::processLocation(Directive* directive)
{
	Directive*		server = nullptr;
	Directive*		indexDirective;
	Location		location = Location();
	std::string		root = "";
	std::string		index = "";

	//	Inherit from server (THESE COULD BE OPTIMIZED BY HAVING THE MAIN FUNCTION DO THIS AND PASSING THE PRE-WORKED LOCATION WITH THESE VALUES, for every location)
	server = getServerDirective(directive);
	if (server == nullptr)
		throw ConfigError::semantics("Location directive is not in any server.", directive);
	root = getRoot(server);
	location.autoindex = getAutoindex(server);

	indexDirective = server->getChild("index");
	if (indexDirective)
		index = indexDirective->getParameter(0);
	else
		index = "index.html";

	location.name = directive->getParameter(0);

	std::vector<Directive*> locationChildren = directive->getChildren();
	for (Directive* child : locationChildren)
	{
		const std::string& name = child->getName();

		if (name == "return")
		{
			location.returnPage = processReturnPage(child);
			continue ;
		}

		if (child->getParameters().empty()) // Would this ever be the case (in our server)?
			continue ;

		if (name == "root")
			root = child->getParameter(0);
		else if (name == "index")
			index = child->getParameter(0);
		else if (name == "autoindex")
		{
			std::string autoindexParam = child->getParameter(0);
			location.autoindex = (autoindexParam == "on" || autoindexParam == "true");
		}
		else if (name == "methods")
		{
			for (const std::string& method : child->getParameters())
			{
				if (method == "GET")
					location.getMethod = true;
				else if (method == "POST")
					location.postMethod = true;
				else if (method == "DELETE")
					location.deleteMethod = true;
			}
			if (location.getMethod == false && location.postMethod == false && location.deleteMethod == false)
			{
				location.getMethod = true;
				location.postMethod = true;
				location.deleteMethod = true;
			}
		}
	}

	//	Build full path
	location.dirPath = joinPath(root, location.name);
	if (!index.empty())
		location.indexPath = joinPath(location.dirPath, index);


	return (location);
}

/**
 * @brief
 * 
 * @return
 */
int	ConfigFile::processKeepaliveTimeout(Directive* directive)
{
	if (directive)
		return (std::stoi(directive->getParameter(0)));
	else
		return (DEFAULT_KEEP_ALIVE_TIMEOUT);
}

/**
 * @brief
 * 
 * @return
 */
void	ConfigFile::processErrorPages(Directive* directive, std::unordered_map<int, ErrorPage>& errorPages)
{
	Directive*	serverDirective = directive->getParent();
	std::string	URI;
	bool		isRedirect = false;
	int			redirectCode = -1;
	int			numErrorCodes;
	std::string	root;

	// Include default error pages with codes 4 & 5.
	root = getRoot(serverDirective);
	root = joinPath(root, DEFAULT_ERROR_PAGES_PATH);
	errorPages.emplace(DEFAULT_40x_ERROR_CODE, ErrorPage(DEFAULT_40x_ERROR_CODE, joinPath(root, DEFAULT_40x_ERROR_PAGE)));
	errorPages.emplace(DEFAULT_50x_ERROR_CODE, ErrorPage(DEFAULT_50x_ERROR_CODE, joinPath(root, DEFAULT_50x_ERROR_PAGE)));

	numErrorCodes = directive->getParameters().size() - 1;
	URI = directive->getParameter(numErrorCodes);

	// Check for redirect syntax (=code) - only if we have at least 3 parameters
	if (numErrorCodes > 1 && directive->getParameter(numErrorCodes - 1).at(0) == '=')
	{
		isRedirect = true;
		redirectCode = std::stoi(directive->getParameter(numErrorCodes - 1).substr(1));
		numErrorCodes -= 1;
	}
	for (size_t i = 0; i < numErrorCodes; i++)
	{
		ErrorPage	current;
		current.errorCode = std::stoi(directive->getParameter(i));
		current.isRedirect = isRedirect;
		current.redirectCode = redirectCode;
		current.URI = URI;
		errorPages.emplace(current.errorCode, current);
	}
}

// If the page is a link, the page must be a valid path and have reading access.
// If the link exists but has no reading access it throws an error,
// Otherwise the string is considered a page by itself.
ReturnPage	ConfigFile::processReturnPage(const Directive* directive)
{
	ReturnPage	returnPage;
	struct stat	st;

	returnPage.code = std::stoi(directive->getParameter(0));
	if (directive->getParameters().size() == 1)
		return (returnPage);
	
	returnPage.page = directive->getParameter(1);
	if (stat(std::string(returnPage.page).c_str(), &st) == 0)
	{
		if (access(returnPage.page.c_str(), R_OK) != 0)
			throw ConfigError::semantics("Return page: " + returnPage.page + " has no reading access.", directive);
		returnPage.isURI = true;
	}
	return (returnPage);
}
