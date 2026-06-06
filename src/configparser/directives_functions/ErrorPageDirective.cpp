#include "../Configuration.hpp"

// Figure out how we want to change the error code... where do we want to store that data.
void	validateErrorPageDirective(Directive* node)
{
	// error_page 404 /404.html;			The last value is a URI.
	// error_page 500 502 503 /50x.html;	There can be multiple codes.
	// error_page 404 =200 /empty.gif;		An equals changes the code.

	// Last parameter is always the URI
	const std::string&	uri = node->getParameters().back();
	std::string			root = getRoot(node);
	std::string			errorPagePath = joinPath(joinPath(root, "error_pages"), uri);

	checkPath(errorPagePath, ErrorType::VALIDATOR, "Error page: " + errorPagePath, false);
	node->setParameter(node->getParameters().size() - 1, errorPagePath);

	// All parameters except the last one are error codes
	for (size_t i = 0; i < node->getParameters().size() - 1; ++i)
	{
		const std::string& param = node->getParameter(i);

		// Check for response code change (e.g., "=200")
		if (param[0] == '=')
		{
			// std::cout << "DEBUG: ErrorPageDirective: " << param << std::endl;
			if (param.length() < 2)
				throw ConfigError::validation("Invalid response code change in " + node->getName() + " directive: '" + param + "'", node);
			
			int newCode = std::stoi(param.substr(1)); // In try-catch block to throw a different error than the one from stoi.
			// std::cout << "DEBUG: New code is: " << newCode << std::endl;
			// Check it's a valid HTTP status code
			if (newCode < 100 || newCode > 599)
				throw ConfigError::validation("Invalid HTTP status code in " + node->getName() + " directive: '" + param + "' must be between 100-599", node);
		}
		else
		{
			// Regular error code
			int errorCode = std::stoi(param);
			// Must be a 4xx or 5xx error code
			if (errorCode < 400 || errorCode > 599)
				throw ConfigError::validation("Invalid error code in " + node->getName() + " directive: '" + param + "' must be between 400-599", node);
		}
	}
	return ;
}

/**
 * @brief
 * 
 * @return
 */
void	ConfigFile::processErrorPages(Directive* directive, std::unordered_map<int, ErrorPage>& errorPages)
{
	std::string	URI;
	bool		isRedirect = false;
	int			redirectCode = -1;
	int			numErrorCodes;

	// Codes without a configured error_page fall back to a generated default
	// page at serve time, so nothing is seeded here.
	numErrorCodes = directive->getParameters().size() - 1;
	URI = directive->getParameter(numErrorCodes);

	// Check for redirect syntax (=code) - only if we have at least 3 parameters
	if (numErrorCodes > 1 && directive->getParameter(numErrorCodes - 1).at(0) == '=')
	{
		isRedirect = true;
		redirectCode = std::stoi(directive->getParameter(numErrorCodes - 1).substr(1));
		numErrorCodes -= 1;
	}
	for (size_t i = 0; i < numErrorCodes; i++)
	{
		ErrorPage	current;
		current.errorCode = std::stoi(directive->getParameter(i));
		current.isRedirect = isRedirect;
		current.redirectCode = redirectCode;
		current.URI = URI;
		errorPages.emplace(current.errorCode, current);
	}
}
