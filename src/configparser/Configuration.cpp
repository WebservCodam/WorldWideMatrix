#include "Configuration.hpp"

ConfigFile::ConfigFile(std::vector<std::unique_ptr<Directive>> directives) : _directives(std::move(directives)) {}

const Directive*	ConfigFile::findDirective(const std::string& name) const
{
	for (const std::unique_ptr<Directive>& directive : this->_directives)
	{
		if (directive && directive->getName() == name)
			return (directive.get()); // Extracts raw pointer from unique_ptr (test if this is a dangling pointer on return...)
	}
	return (nullptr);
}

std::vector<Directive*>	ConfigFile::findAllDirectives(const std::string& name) const
{
	std::vector<Directive*>	result;

	for (size_t i = 0; i < this->_directives.size(); ++i)
	{
		const std::unique_ptr<Directive>& directive = this->_directives[i];

		if (!directive)
			continue ;

		if (directive->getName() == name)
			result.push_back(directive.get());
	}
	return (result);
}

std::vector<std::unique_ptr<Directive>>&	ConfigFile::getDirectives()
{
	return (this->_directives);
}

const std::vector<ServerConfig>&	ConfigFile::getServers() const
{
	return (this->_servers);
}

const ServerConfig&	ConfigFile::getServer(const std::string& serverName)
{
	for (const ServerConfig& server : this->_servers)
	{
		if (server.getServerName() == serverName)
			return (server);
	}
	throw std::runtime_error("ServerConfig with name '" + serverName + "' not found");
}

std::vector<ServerConfig>	ConfigFile::createServers()
{
	std::vector<Directive*> serverDirectives = this->findAllDirectives("server");

	for (Directive* serverDirective : serverDirectives)
	{
		if (serverDirective->getName() != "server")
			continue ;

		std::string							serverName;
		std::vector<ListenDirective>		listenDirectives;
		unsigned long long					maxBodySize;
		std::unordered_map<int, ErrorPage>	errorPages;
		std::vector<Location>				locations;
		int									keepalive_timeout;

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
				errorPages = processErrorPages(directive);
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
}

/**
 * @brief
 * 
 */
Location	ConfigFile::processLocation(Directive* directive)
{
	Directive*		server;
	Location		location;
	std::string		root = "";
	std::string		index = "";

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

		if (child->getParameters().empty())
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

	//	Inherit from server if not set locally
	server = directive->getParent();
	if (root.empty())
	{
		if (server && server->getChild("root"))
			root = server->getChild("root")->getParameter(0);
		else
			root = "/www/";
	}
	if (index.empty() && location.autoindex == false)
	{
		if (server && server->getChild("index"))
			index = server->getChild("index")->getParameter(0);
		else
			index = "index.html";
	}

	//	Build full path
	location.dirPath = joinPath(root, location.name);
	if (!index.empty())
		location.indexPath = joinPath(location.dirPath, index);

	// CHECK THE PATH BEFORE RETURN

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
std::unordered_map<int, ErrorPage>	ConfigFile::processErrorPages(const Directive* directive)
{
	std::unordered_map<int, ErrorPage>	errorPages;
	std::string							URI;
	bool								isRedirect;
	int									redirectCode = -1;
	int									numErrorCodes;  // This is the number of error codes.

	// std::cout << "DEBUG: processErrorPages" << std::endl;

	numErrorCodes = directive->getParameters().size() - 1;
	// std::cout << "DEBUG: number of error codes: " + std::to_string(numErrorCodes) << std::endl;
	URI = directive->getParameter(numErrorCodes);
	// std::cout << "DEBUG: URI: " + URI << std::endl;
	if (directive->getParameter(numErrorCodes - 1).at(0) == '=')
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
	return (errorPages);
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
	
	returnPage.page = std::stoi(directive->getParameter(1));
	if (stat(std::string(returnPage.page).c_str(), &st) == 0)
	{
		if (access(returnPage.page.c_str(), R_OK) != 0)
			throw ConfigError::semantics("Return page: " + returnPage.page + " has no reading access.", directive);
		returnPage.isURI = true;
	}
	return (returnPage);
}


// ----- DIRECTIVE CLASS -----

Directive::Directive(
    size_t line,
    size_t column,
    const std::string& name,
    const std::string& context,
    std::vector<std::string> parameters,
    std::vector<std::unique_ptr<Directive>> children)
    : _line(line),
      _column(column),
      _name(name),
      _context(context),
      _parameters(std::move(parameters)),
      _children(std::move(children))
{}

// ----- GETTERS ------

size_t	Directive::getLine() const
{
	return (this->_line);
}

size_t	Directive::getColumn() const
{
	return (this->_column);
}

const std::string&	Directive::getName() const
{
	return (this->_name);
}

const std::string&	Directive::getContext() const
{
	return (this->_context);
}

const std::string&	Directive::getParameter(size_t i) const
{
	if (i < 0 || i >= _parameters.size())
		throw std::out_of_range("Parameter index out of range");

	return (_parameters.at(i));
}

const std::vector<std::string>&	Directive::getParameters() const
{
	return (this->_parameters);
}

Directive*	Directive::getChild(size_t i)
{
	if (i < 0 || i >= _children.size())
		return (nullptr);

	return (_children[i].get());
}

Directive*	Directive::getChild(const std::string& name)
{
	for (const std::unique_ptr<Directive>& child : _children)
	{
		if (child && child->getName() == name)
			return (child.get());
	}
	return (nullptr); // Throw error in the calling function.
}

std::vector<Directive*>	Directive::getChildren()
{
	std::vector<Directive*>	result;

	result.reserve(_children.size());
	for (const std::unique_ptr<Directive>& child : _children)
	{
		result.push_back(child.get());
	}
	return (result);
}

Directive*	Directive::getParent()
{
	return (this->_parent);
}

// ----- SETTERS -----

void	Directive::setLine(size_t line)
{
	this->_line = line;
}

void	Directive::setColumn(size_t column)
{
	this->_column = column;
}
void	Directive::setName(const std::string& name)
{
	this->_name = name;
}
void	Directive::setContext(const std::string& context)
{
	this->_context = context;
}

void	Directive::setParameter(int index, const std::string& new_parameter)
{
	this->_parameters.at(index) = new_parameter;
}

void	Directive::setParameters(const std::vector<std::string>& parameters)
{
	this->_parameters = parameters;
}

void	Directive::addChild(std::unique_ptr<Directive> child)
{
	this->_children.push_back(std::move(child));
}

void	Directive::setParent(Directive* parent)
{
	this->_parent = parent;
}
