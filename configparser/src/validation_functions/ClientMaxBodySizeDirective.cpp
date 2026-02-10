#include "../../include/Configuration.hpp"

bool	validateClientMaxBodySizeDirective(Directive* node)
{
	// If 0 == no limit
	// Can be 1m or 10m which means 10 megabytes. Any request that exceeds that it returns a 413 error.
	// The letter can be lower or upper case.
	// It can also be a number

	if (node->getParameters().empty())
		throw ConfigError::validation("Directive '" + node->getName() + "' requires a parameter", node);	// I belive this is checked in requiredArguments, so it should never enter this condition.
	if (node->getParameter(0) == "0")
		return (true);

	try
	{
		const std::string& param = node->getParameter(0);
		if (param.empty())
			throw ConfigError::validation("Invalid size value in '" + node->getName() + "' directive", node);

		// Check if the string is purely a number.
		bool	isNumber = false;
		for (char c : param)
		{
			if (!std::isdigit(c))
				break ;
			if (c == param.back())
				isNumber = true;
		}

		if (isNumber)
		{
			
		}

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
