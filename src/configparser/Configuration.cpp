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
			continue;

		std::string							serverName;
		std::vector<ListenDirective>		listenDirectives;
		unsigned long long					maxBodySize;
		std::unordered_map<int, ErrorPage>	errorPages;
		std::vector<Location>				locations;
		int									keepalive_timeout;

		std::vector<Directive*> serverChildren = serverDirective->getChildren();

		for (Directive* directive : serverChildren)
		{
			if (!directive) {
				continue;
			}

			if (directive->getName() == "server_name")
				serverName = processServerName(directive);
			else if (directive->getName() == "listen")
				processListen(directive, listenDirectives);
			else if (directive->getName() == "client_max_body_size")
				processClientMaxBodySize(directive, maxBodySize);
			else if (directive->getName() == "error_page")
				errorPages = processErrorPages(directive);
			else if (directive->getName() == "location")
				locations.push_back(processLocation(directive));
			else if (directive->getName() == "keepalive_timeout")
				processKeepaliveTimeout(directive, keepalive_timeout);
		}

		ServerConfig server(serverName, listenDirectives, maxBodySize, errorPages, locations, keepalive_timeout);
		this->_servers.push_back(server);
	}

	return (this->_servers);
}

std::string	ConfigFile::processServerName(const Directive* directive)
{
	if (directive && !directive->getParameters().empty())
		return (directive->getParameter(0));
	return ("default");
}

void	ConfigFile::processListen(const Directive* directive, std::vector<ListenDirective>& listenDirectives)
{
	if (directive && !directive->getParameters().empty())
	{
		std::string listenParam = directive->getParameter(0);
		size_t colonPos = listenParam.find(':');

		if (colonPos != std::string::npos)
		{
			std::string address = listenParam.substr(0, colonPos);
			std::string port = listenParam.substr(colonPos + 1);
			listenDirectives.push_back(ListenDirective(address, port));
		}
		else
		{
			// If no colon, treat the parameter as a port number (default address: 0.0.0.0)
			listenDirectives.push_back(ListenDirective("0.0.0.0", listenParam));
		}
	}
}

void	ConfigFile::processClientMaxBodySize(const Directive* directive, unsigned long long& maxBodySize)
{
	if (directive && !directive->getParameters().empty())
	{
		std::string sizeStr = directive->getParameter(0);
		try {
			maxBodySize = std::stoull(sizeStr);
		} catch (const std::exception&) {
			maxBodySize = 2000000; // Defaults to 2MB
		}
	}
}

Location	ConfigFile::processLocation(Directive* directive)
{
	if (!directive || directive->getParameters().empty())
		return Location("/"); // What is this? and Why?

	std::string		path = directive->getParameter(0);
	std::string		root = "";
	std::string		index = "";
	bool			autoindex = false;
	bool			getMethod = false;
	bool			postMethod = false;
	bool			deleteMethod = false;
	ReturnPage		returnPage;

	std::vector<Directive*> locationChildren = directive->getChildren();

	for (Directive* child : locationChildren)
	{
		if (child->getName() == "root" && !child->getParameters().empty())
			root = child->getParameter(0);
		else if (child->getName() == "index" && !child->getParameters().empty())
			index = child->getParameter(0);
		else if (child->getName() == "autoindex" && !child->getParameters().empty())
		{
			std::string autoindexParam = child->getParameter(0);
			autoindex = (autoindexParam == "on" || autoindexParam == "true");
		}
		else if (child->getName() == "methods"  && !child->getParameters().empty())
		{
			// std::cout << "DEBUG: enters elseif statement 'methods'" << std::endl;
			getMethod = false;
			postMethod = false;
			deleteMethod = false;

			for (const std::string& method : child->getParameters())
			{
				if (method == "GET")
					getMethod = true;
				else if (method == "POST")
					postMethod = true;
				else if (method == "DELETE")
					deleteMethod = true;
			}
		}
		else if (child->getName() == "return")
		{
			returnPage = processReturnPage(child);
		}
	}

	return Location(path, root, index, autoindex, getMethod, postMethod, deleteMethod, returnPage);
}

void	ConfigFile::processKeepaliveTimeout(Directive* directive, int& keepalive_timeout)
{
	if (directive && !directive->getParameters().empty())
	{
		std::string sizeStr = directive->getParameter(0);
		try {
			keepalive_timeout = std::stoi(sizeStr);
		} catch (const std::exception&) {
			keepalive_timeout = 30; // Defaults to 30
		}
	}
}

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
		return (nullptr); // Throw exception

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
