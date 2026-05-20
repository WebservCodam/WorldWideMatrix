#include "../Configuration.hpp"

void	validateListenDirective(Directive* node)
{

	// std::cout << "DEBUG: In validateListenDirective" << std::endl;

	std::pair<std::string, std::string>	addressAndPort;	//127.0.0.0:8080?
	bool								isValidAddress;
	bool								isValidPort;

	// if (node->getParameters().empty())
	// 	throw ConfigError::validation("Directive '" + node->getName() + "' requires at least one parameter", node);

	addressAndPort = parseAddressAndPort(node->getParameter(0));
	if (addressAndPort.first.empty() && addressAndPort.second.empty())
	{
		if (validateAddress(node->getParameter(0)))
			return ;
		if (validatePort(node->getParameter(0)))
			return ;
		throw ConfigError::validation("Invalid address or port format in '" + node->getName() + "' directive: '" + node->getParameter(0) + "'", node);
	}
	isValidAddress = validateAddress(addressAndPort.first);
	isValidPort = validatePort(addressAndPort.second);
	if (isValidAddress && isValidPort)
		return ;
	else if (isValidAddress && addressAndPort.second.empty())
	{
		node->setParameter(0, addressAndPort.first);
		return ;
	}
	else if (addressAndPort.first.empty() && isValidPort)
	{
		node->setParameter(0, addressAndPort.second);
		return ;
	}
	throw ConfigError::validation("Invalid address:port combination in '" + node->getName() + "' directive: '" + node->getParameter(0) + "'", node);
}

void	ConfigFile::checkUsedPorts(const Directive* directive, const std::string& port)
{
	for (const std::string& usedPort : this->_usedPorts)
	{
		if (port == usedPort)
			throw ConfigError::validation("The port '" + port + "' is already being used.", directive);
	}
	this->_usedPorts.push_back(port);
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
			checkUsedPorts(directive, port);
			listenDirectives.push_back(ListenDirective(address, port));
		}
		else
		{
			if (validateAddress(listenParam))
			{
				checkUsedPorts(directive, "8080");
				listenDirectives.push_back(ListenDirective(listenParam, "8080"));
			}
			else if (validatePort(listenParam))
			{
				checkUsedPorts(directive, listenParam);
				listenDirectives.push_back(ListenDirective("127.0.0.1", listenParam));
			}
			else
				throw ConfigError::semantics("Invalid address & port", directive);
		}
	}
}
