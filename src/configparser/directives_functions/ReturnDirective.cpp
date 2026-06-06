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
	int					paramsSize = node->getParameters().size();
	const std::string&	statusStr = node->getParameter(0);
	int					statusCode = std::stoi(statusStr);

	// Must be a valid HTTP status code (100-599)
	if (statusCode < 100 || statusCode > 599)
		throw ConfigError::validation("Invalid HTTP status code in " + node->getName() + " directive: '" + statusStr + "' must be between 100-599", node);

	// std::cout << "DEBUG: Testing throwing error in validateReturn" << std::endl;
	// throw ConfigError::validation("TEST", node);

	// A second parameter is the redirect target, only valid for 3xx codes.
	if (paramsSize == 2)
	{
		if (statusCode < 300 || statusCode > 399)
			throw ConfigError::validation("Second parameter in " + node->getName() + " directive is only allowed with a 3xx redirect status code.", node);

		const std::string& secondParam = node->getParameter(1);

		if (secondParam.empty())
			throw ConfigError::validation("Second parameter in " + node->getName() + " is empty.", node);

		// Validate the redirect target as a path.
		for (size_t i = 0; i < secondParam.length(); ++i)
		{
			char c = secondParam[i];
			// Allow alphanumeric, slash, dash, underscore, dot, percent (for URL encoding)
			if (!std::isalnum(c) && c != '/' && c != '-' && c != '_' && c != '.' && c != '%')
				throw ConfigError::validation("Invalid path character in " + node->getName() + " directive: '" + secondParam + "'", node);
		}

		// Check for consecutive slashes (except at start)
		if (secondParam.find("//") != std::string::npos)
			throw ConfigError::validation("Invalid path in " + node->getName() + " directive: '" + secondParam + "' contains consecutive slashes", node);
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
