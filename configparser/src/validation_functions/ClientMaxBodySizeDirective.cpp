#include "../../include/Configuration.hpp"

bool	validateClientMaxBodySizeDirective(Directive* node)
{
	// If 0 == no limit
	// Can be 1m or 10m which means 10 megabytes. Any request that exceeds that it returns a 413 error.
	// The letter can be lower or upper case.
	// It can also be a number

	if (node->getParameters().empty())
		throw ConfigError::validation("Directive '" + node->getName() + "' requires a parameter", node);
	if (node->getParameter(0) == "0")
		return (true);

	std::cout << "DEBUG in validateClientBodySize - Parameter is: " << node->getParameter(0) << std::endl;
	node->setParameter(0, "20");
	std::cout << "DEBUG in validateClientBodySize - the new Parameter is: " << node->getParameter(0) << std::endl;

	try
	{
		const std::string& param = node->getParameter(0);
		if (param.empty())
			throw ConfigError::validation("Invalid size value in '" + node->getName() + "' directive", node);

		char lastChar = param.back(); // IT CAN ALSO BE JUST A NUMBER!
		if (lastChar != 'm' && lastChar != 'M')
			throw ConfigError::validation("Invalid size unit in '" + node->getName() + "' directive: '" + param + "' (must end with 'm' or 'M')", node);

		std::string numPart = param.substr(0, param.length() - 1);
		int	size = std::stoi(numPart);
		if (size < 1 || size > 10)
			throw ConfigError::validation("Invalid size value in '" + node->getName() + "' directive: '" + param + "' (must be between 1m and 10m)", node);
	}
	catch(const ConfigError&)
	{
		throw ; // Rethrows the config error, outside of the function now.
	}
	catch(const std::exception& e)
	{
		throw ConfigError::validation("Invalid size format in '" + node->getName() + "' directive: '" + node->getParameter(0) + "'", node);
	}
	return (true);
}
