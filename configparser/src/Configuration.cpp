#include "../include/Configuration.hpp"

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

std::vector<const Directive*>	ConfigFile::findAllDirectives(const std::string& name) const
{
	std::vector<const Directive*>	result;

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

const std::vector<std::unique_ptr<Directive>>&	ConfigFile::getDirectives() const
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
	std::vector<const Directive*> serverDirectives = this->findAllDirectives("server");

	for (const Directive* serverDirective : serverDirectives)
	{
		if (!serverDirective) {
			continue;
		}

		if (serverDirective->getName() != "server")
			continue;

		std::string							serverName = "default_server";
		std::vector<ListenDirective>		listenDirectives;	//The structure already defaults to 0.0.0.0:8080
		size_t								maxBodySize = 1048576;
		std::map<int, std::string>			errors;
		std::vector<Location>				locations;

		std::vector<const Directive*> serverChildren = serverDirective->getChildren();

		for (const Directive* directive : serverChildren)
		{
			if (!directive) {
				continue;
			}

			if (directive->getName() == "server_name")
				processServerName(directive, serverName);
			else if (directive->getName() == "listen")
				processListen(directive, listenDirectives);
			else if (directive->getName() == "client_max_body_size")
				processClientMaxBodySize(directive, maxBodySize);
			else if (directive->getName() == "error_page")
				processErrorPage(directive, errors);
			else if (directive->getName() == "location")
				locations.push_back(processLocation(directive));
		}

		ServerConfig server(serverName, listenDirectives, maxBodySize, errors, locations);
		this->_servers.push_back(server);
	}

	return (this->_servers);
}

void	ConfigFile::processServerName(const Directive* directive, std::string& serverName)
{
	if (directive && !directive->getParameters().empty())
		serverName = directive->getParameter(0);
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

void	ConfigFile::processClientMaxBodySize(const Directive* directive, size_t& maxBodySize)
{
	if (directive && !directive->getParameters().empty())
	{
		std::string sizeStr = directive->getParameter(0);
		try {
			maxBodySize = std::stoull(sizeStr);
		} catch (const std::exception&) {
			maxBodySize = 1048576;
		}
	}
}

void	ConfigFile::processErrorPage(const Directive* directive, std::map<int, std::string>& errors)
{
	if (directive && directive->getParameters().size() >= 2)
	{
		std::string	errorPagePath = directive->getParameter(directive->getParameters().size() - 1);

		for (size_t i = 0; i < directive->getParameters().size() - 1; ++i)
		{
			try {
				int code = std::stoi(directive->getParameter(i));
				errors[code] = errorPagePath;
			} catch (const std::exception&) {
				continue;
			}
		}
	}
}

Location	ConfigFile::processLocation(const Directive* directive)
{
	if (!directive || directive->getParameters().empty())
		return Location("/");

	std::string		path = directive->getParameter(0);
	std::string		root = "";
	std::string		index = "";
	bool			autoindex = false;
	bool			getMethod = false;
	bool			postMethod = false;
	bool			deleteMethod = false;

	std::vector<const Directive*> locationChildren = directive->getChildren();

	for (const Directive* locationDirective : locationChildren)
	{
		if (!locationDirective)
			continue ;

		if (locationDirective->getName() == "root")
		{
			if (!locationDirective->getParameters().empty())
				root = locationDirective->getParameter(0);
		}
		else if (locationDirective->getName() == "index")
		{
			if (!locationDirective->getParameters().empty())
				index = locationDirective->getParameter(0);
		}
		else if (locationDirective->getName() == "autoindex")
		{
			if (!locationDirective->getParameters().empty())
			{
				std::string autoindexParam = locationDirective->getParameter(0);
				autoindex = (autoindexParam == "on" || autoindexParam == "true");
			}
		}
		else if (locationDirective->getName() == "allow_methods")
		{
			// std::cout << "DEBUG: enters elseif statement 'allow_methods'" << std::endl;
			getMethod = false;
			postMethod = false;
			deleteMethod = false;

			for (const std::string& method : locationDirective->getParameters())
			{
				// std::cout << "DEBUG: method is " << method << std::endl;
				if (method == "GET")
				{
					getMethod = true;
					// std::cout << "DEBUG: GET set to true" << std::endl;
				}
				else if (method == "POST")
				{
					postMethod = true;
					// std::cout << "DEBUG: POST set to true" << std::endl;
				}
				else if (method == "DELETE")
				{
					deleteMethod = true;
					// std::cout << "DEBUG: DELETE set to true" << std::endl;
				}
			}
		}
	}

	return Location(path, root, index, autoindex, getMethod, postMethod, deleteMethod);
}

// --- DIRECTIVE CLASS ---

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

// Getters

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

const Directive*	Directive::getChild(size_t i) const
{
	if (i < 0 || i >= _children.size())
		return (nullptr); // Throw exception

	return (_children[i].get());
}

std::vector<const Directive*>	Directive::getChildren() const
{
	std::vector<const Directive*>	result;

	result.reserve(_children.size());
	for (const std::unique_ptr<Directive>& child : _children)
	{
		result.push_back(child.get());
	}
	return (result);
}

// Setters

void Directive::setLine(size_t line)
{
	this->_line = line;
}

void Directive::setColumn(size_t column)
{
	this->_column = column;
}
void Directive::setName(const std::string& name)
{
	this->_name = name;
}
void Directive::setContext(const std::string& context)
{
	this->_context = context;
}
void Directive::setParameters(const std::vector<std::string>& parameters)
{
	this->_parameters = parameters;
}

void Directive::addChild(std::unique_ptr<Directive> child)
{
	this->_children.push_back(std::move(child));
}
