#include "../Configuration.hpp"

/**
 * @brief
 * 
 * @return
 */
void	validateReturnDirective(Directive* node)
{
	// return 301 /uri;   (3xx codes take a redirect target)
	// return 404;        (other codes take no second parameter)

	// std::cout << "DEBUG: Testing in validateReturn" << std::endl;

	// First parameter must be a valid HTTP status code
	int	paramsSize = node->getParameters().size();
	int status_code = std::stoi(node->getParameter(0));

	// Must be a valid HTTP status code (100-599)
	if (status_code < 100 || status_code > 599)
		throw ConfigError::validation("Invalid HTTP status code in " + node->getName() + " directive: '" + node->getParameter(0) + "' must be between 100-599", node);

	// std::cout << "DEBUG: Testing throwing error in validateReturn" << std::endl;
	// throw ConfigError::validation("TEST", node);

	// A second parameter is the redirect target, only valid for 3xx codes.
	if (paramsSize == 2)
	{
		if (status_code < 300 || status_code > 399)
			throw ConfigError::validation("Second parameter in " + node->getName() + " directive is only allowed with a 3xx redirect status code.", node);

		const std::string& second_param = node->getParameter(1);

		if (second_param.empty())
			throw ConfigError::validation("Second parameter in " + node->getName() + " is empty.", node);

		// Validate the redirect target as a path.
		for (size_t i = 0; i < second_param.length(); ++i)
		{
			char c = second_param[i];
			// Allow alphanumeric, slash, dash, underscore, dot, percent (for URL encoding)
			if (!std::isalnum(c) && c != '/' && c != '-' && c != '_' && c != '.' && c != '%')
				throw ConfigError::validation("Invalid path character in " + node->getName() + " directive: '" + second_param + "'", node);
		}

		// Check for consecutive slashes (except at start)
		if (second_param.find("//") != std::string::npos)
			throw ConfigError::validation("Invalid path in " + node->getName() + " directive: '" + second_param + "' contains consecutive slashes", node);
	}
}


// Stores the status code and, for a 3xx redirect, the target path.
ReturnPage	ConfigFile::processReturnPage(const Directive* directive)
{
	ReturnPage	returnPage;

	returnPage.code = std::stoi(directive->getParameter(0));
	if (directive->getParameters().size() == 2)
		returnPage.uri = directive->getParameter(1);
	return (returnPage);
}
