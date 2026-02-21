#include "../Configuration.hpp"

// bool	validateAllowOrDenyDirective(Directive* node)
// {
// 	const std::string&	address = node->getParameters().at(0);
// 	size_t				cidrPos = 0;
// 	std::string			addressPart;
// 	std::string			cidrPart;

// 	if (address.empty())
// 		throw ConfigError::validation("Directive '" + node->getName() + "' requires a non-empty address parameter", node);

// 	if (address == "all")
// 		return (true);

// 	cidrPos = address.find("/");
// 	if (cidrPos != std::string::npos)
// 	{
// 		addressPart = address.substr(0, cidrPos);
// 		cidrPart = address.substr(cidrPos + 1);
// 		try
// 		{
// 			int	cidr = std::stoi(cidrPart);
// 			if (cidrPart.length() != std::to_string(cidr).length())
// 				throw ConfigError::validation("Invalid CIDR notation in '" + node->getName() + "': '" + address + "'", node);
// 			if (cidr < 0 || cidr > 32)
// 				throw ConfigError::validation("CIDR value must be between 0 and 32 in '" + node->getName() + "': '" + address + "'", node);
// 		}
// 		catch(const ConfigError&)
// 		{
// 			throw;
// 		}
// 		catch(const std::exception& e)
// 		{
// 			throw ConfigError::validation("Invalid CIDR value in '" + node->getName() + "': '" + address + "'", node);
// 		}
// 		if (!validateAddress(addressPart))
// 			throw ConfigError::validation("Invalid IP address in '" + node->getName() + "': '" + addressPart + "'", node);
// 	}
// 	else
// 	{
// 		if (!validateAddress(address))
// 			throw ConfigError::validation("Invalid IP address in '" + node->getName() + "': '" + address + "'", node);
// 	}
// 	return (true);
// }
