#include "../../include/Configuration.hpp"

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
	if (isValidAddress && addressAndPort.second.empty())
		return ;
	else if (isValidAddress && isValidPort)
		return ;
	else if (addressAndPort.first.empty() && isValidPort)
		return ;
	throw ConfigError::validation("Invalid address:port combination in '" + node->getName() + "' directive: '" + node->getParameter(0) + "'", node);
}
