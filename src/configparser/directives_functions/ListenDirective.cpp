#include "../Configuration.hpp"

void	validateListenDirective(Directive* node)
{
	const std::string&					hostPort = node->getParameter(0);
	std::pair<std::string, std::string>	addressAndPort;
	bool								isValidAddress;
	bool								isValidPort;

	addressAndPort = parseAddressAndPort(hostPort);
	if (addressAndPort.first.empty() && addressAndPort.second.empty())
	{
		if (validateAddress(hostPort))
			return ;
		if (validatePort(hostPort))
			return ;
		throw ConfigError::validation("Invalid address or port format in '" + node->getName() + "' directive: '" + hostPort + "'", node);
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
	throw ConfigError::validation("Invalid address:port combination in '" + node->getName() + "' directive: '" + hostPort + "'", node);
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
			{
				listenDirectives.push_back(ListenDirective(listenParam, "8080"));
			}
			else if (validatePort(listenParam))
			{
				listenDirectives.push_back(ListenDirective("127.0.0.1", listenParam));
			}
			else
				throw ConfigError::semantics("Invalid address & port", directive);
		}
	}
}
