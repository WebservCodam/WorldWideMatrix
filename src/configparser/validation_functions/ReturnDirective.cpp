#include "../Configuration.hpp"

/**
 * @brief
 * 
 * @return
 */
void	validateReturnDirective(Directive* node)
{
	// return 301 URI;
	// return 404;
	// return 200 "some text";

	// std::cout << "DEBUG: Testing in validateReturn" << std::endl;

	// First parameter must be a valid HTTP status code
	int	paramsSize = node->getParameters().size();
	int status_code = std::stoi(node->getParameter(0));

	// Must be a valid HTTP status code (100-599)
	if (status_code < 100 || status_code > 599)
		throw ConfigError::validation("Invalid HTTP status code in " + node->getName() + " directive: '" + node->getParameter(0) + "' must be between 100-599", node);

	// std::cout << "DEBUG: Testing throwing error in validateReturn" << std::endl;
	// throw ConfigError::validation("TEST", node);

	// If there's a second parameter, validate it as URL or text
	if (paramsSize == 2)
	{
		const std::string& second_param = node->getParameter(1);

		if (second_param.empty())
			throw ConfigError::validation("Second parameter in " + node->getName() + " is empty.", node);
		
		// For quoted text
		if (second_param.front() == '"' && second_param.back() == '"')
		{
			// Must have content between quotes
			if (second_param.length() < 3)
				throw ConfigError::validation("Invalid quoted text in " + node->getName() + " directive: must have content between quotes", node);
		}
		// For relative URLs or paths
		else
		{
			// Check for valid path characters and format
			for (size_t i = 0; i < second_param.length(); ++i)
			{
				char c = second_param[i];
				// Allow alphanumeric, slash, dash, underscore, dot, percent (for URL encoding)
				if (!std::isalnum(c) && c != '/' && c != '-' && c != '_' && c != '.' && c != '%')
					throw ConfigError::validation("Invalid path character in " + node->getName() + " directive: '" + second_param + "'", node);
			}

			// Check for consecutive slashes (except at start)
			if (second_param.find("//") != std::string::npos)
				throw ConfigError::validation("Invalid path in " + node->getName() + "  directive: '" + second_param + "' contains consecutive slashes", node);
		}
	}
}
