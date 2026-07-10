#include "Configuration.hpp"

std::string	trimPathName(const std::string& name)
{
	std::size_t len = name.length();

	if (name.front() == '/' && name.back() == '/' && len > 1)
		return (name.substr(1, len - 2));
	if (name.front() == '/' && len > 1)
		return (name.substr(1));
	if (name.back() == '/' && len > 1)
		return(name.substr(0, len - 1));
	return (name);
}

std::string	joinPath(const std::string& a, const std::string& b)
{
	if (a.empty())
		return (b);
	if (b.empty())
		return (a);
	if (a.back() == '/' && b.front() == '/')
		return (a + b.substr(1));
	if (a.back() != '/' && b.front() != '/')
		return (a + "/" + b);
	return (a + b);
}

void	checkPath(const std::string& path, ErrorType errorType, const std::string& msg, bool checkDir)
{
	struct stat	st;

	// Check URIs
	if (stat(std::string(path).c_str(), &st) != 0)
		throw ConfigError(errorType, msg + std::string(" doesn't exist."));
	if (checkDir == true && !S_ISDIR(st.st_mode))
		throw ConfigError(errorType, msg + std::string(" is not a directory."));
	if (access(path.c_str(), R_OK) != 0)
		throw ConfigError(errorType, msg + std::string(" doesn't have reading or executing permissions."));
}

Directive*	getServerDirective(Directive *node)
{
	for (Directive* iterator = node; iterator != nullptr ; iterator = iterator->getParent())
	{
		if (iterator->getName() == "server")
			return (iterator);
	}
	return (nullptr);
}

bool	getAutoindex(Directive *node)
{
	Directive*	autoindexDirective;
	std::string	autoindexParam = "";

	if (node->getName() == "autoindex")
		autoindexDirective = node;
	else if (node->getName() == "server")
		autoindexDirective = node->getChild("autoindex");
	else
		autoindexDirective = getServerDirective(node)->getChild("autoindex");
	if (autoindexDirective)
		autoindexParam = autoindexDirective->getParameter(0);
	return (autoindexParam == "on" || autoindexParam == "true");
}

std::string	getRoot(Directive *node)
{
	Directive*	rootDirective = nullptr;
	std::string	root = joinPath(".", DEFAULT_ROOT_PATH);

	if (!node)
		throw ConfigError::semantics("Invalid directive given to getRoot()", node);
	if (node->getName() == "root")
		rootDirective = node;
	else if (node->getChild("root")) // In case directive is either server or location.
		rootDirective = node->getChild("root");
	else
	{
		// Walk up and use the nearest enclosing root: a location's own root
		// takes precedence over the server's.
		for (Directive* iterator = node->getParent(); iterator != nullptr ; iterator = iterator->getParent())
		{
			if (iterator->getChild("root"))
			{
				rootDirective = iterator->getChild("root");
				break ;
			}
		}
	}
	if (rootDirective)
	{
		root = rootDirective->getParameter(0);
		if (root.at(0) != '.')
			root = joinPath(".", root);
		rootDirective->setParameter(0, root);
	}
	return (root);
}

std::pair<std::string, std::string>	parseAddressAndPort(const std::string& address)
{
	std::pair<std::string, std::string>	addressAndPort = {"", ""};
	std::string	addressPart;
	std::string	portPart;
	size_t		pos;

	if (address.empty())
		return (addressAndPort);
	pos = address.find(":");
	if (pos == address.npos)
		return (addressAndPort);
	else
	{
		addressPart = address.substr(0, pos);
		portPart = address.substr(pos + 1, address.size() - (pos + 1));
		addressAndPort = {std::move(addressPart), std::move(portPart)};
	}


	return (addressAndPort);
}

bool	validateAddress(const std::string& address)
{
	size_t		currentPos = 0;
	size_t		nextPos;
	size_t		iterations = 0;
	std::string	currentChunk;

	if (address.empty())
		return (false);

	if (address == "localhost")
		return (true);

	while (currentPos < address.length())
	{
		nextPos = address.find(".", currentPos);
		if (nextPos == std::string::npos)
			currentChunk = address.substr(currentPos);
		else
			currentChunk = address.substr(currentPos, nextPos - currentPos);

		if (!isByte(currentChunk))
			return (false);

		iterations++;
		if (nextPos == std::string::npos)
			break;
		currentPos = nextPos + 1;
	}
	if (iterations != 4)
		return (false);
	return (true);
}

bool	validatePort(const std::string& port)
{
	if (port.empty())
		return (false);
	try
	{
		int portNumber = std::stoi(port);
		if (port.size() != std::to_string(portNumber).length())
			throw (std::runtime_error("Wrong port"));
		if (portNumber >= 1 && portNumber <= 65535)
			return (true);
	}
	catch(const std::exception& e)
	{
		return (false);
	}
	return (false);
}

bool isByte(std::string &number)
{
	if (number.empty())
		return (false);

    try
	{
		int	num = std::stoi(number);
		if (number.size() != std::to_string(num).length())
			return (false);
		if (num > 255 || num < 0)
			return (false);
	}
	catch(const std::exception& e)
	{
		return (false);
	}	
    return (true);
}
