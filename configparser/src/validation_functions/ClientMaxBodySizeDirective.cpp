#include "../../include/Configuration.hpp"

bool	validateClientMaxBodySizeDirective(Directive* node)
{
	// If 0 == no limit
	// Can be 1m or 10m which means 10 megabytes. Any request that exceeds that it returns a 413 error.
	// The letter can be lower or upper case.
	// It can also be a number

	// The default max size could be 2MB, and then perhaps a single location /uploads/largefileupload could handle a bigger client_max_body_size.

	std::cout << "DEBUG in clientMaxBodySize - Param: " << node->getParameter(0) << std::endl;

	const long	DEFAULT_SIZE = 2000000;

	if (node->getParameters().empty())
		throw ConfigError::validation("Directive '" + node->getName() + "' requires a parameter", node);	// I belive this is checked in requiredArguments, so it should never enter this condition.
	if (node->getParameter(0) == "0")
		return (true);

	try
	{
		const std::string&	param = node->getParameter(0);
		long				size;

		if (param.empty())
			throw ConfigError::validation("Invalid size value in '" + node->getName() + "' directive", node);

		// Convert parameter to long.
		try
		{
			size = std::stol(param);
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}

		// Check if the string is purely a number.
		if (std::to_string(size).length() == param.length())
		{
			if (size < DEFAULT_SIZE)
				return (true);
			else
				throw ConfigError::validation("Directive '" + node->getName() + "' exceeds the allowed limit.", node);
		}
		
		// Check if the ending is a valid.
		std::string	ending = param.substr(param.size() - 2);
		char		lastChar = param.back();
		if (ending == "kb" || ending == "KB")
		{
			size *= 1000;
			node->setParameter(0, std::to_string(size));
		}
		else if (ending == "mb" || ending == "MB")
		{
			size *= 1000000;
			node->setParameter(0, std::to_string(size));
		}
		else if (lastChar == 'b' || lastChar == 'B' )
		{
			node->setParameter(0, std::to_string(size));
		}
		else if (lastChar == 'k' || lastChar == 'K' )
		{
			size *= 1000;
			node->setParameter(0, std::to_string(size));
		}
		else if (lastChar == 'm' || lastChar == 'M' )
		{
			size *= 1000000;
			node->setParameter(0, std::to_string(size));
		}

		std::cout << "DEBUG in clientMaxBodySize - Param: " << node->getParameter(0) << std::endl;
		
		if (size < DEFAULT_SIZE)
			return (true);
		else
			throw ConfigError::validation("Directive '" + node->getName() + "' exceeds the allowed limit.", node);
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
