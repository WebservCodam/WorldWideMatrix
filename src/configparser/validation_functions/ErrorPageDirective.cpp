#include "../Configuration.hpp"

// Figure out how we want to change the error code... where do we want to store that data.
void	validateErrorPageDirective(Directive* node)
{
	// error_page 404 /404.html;			The last value is a URI.
	// error_page 500 502 503 /50x.html;	There can be multiple codes.
	// error_page 404 =200 /empty.gif;		An equals changes the code.

	// Last parameter is always the URI
	const std::string&	uri = node->getParameters().back();

	// This could be done better by validating all error pages at once...
	Directive*			rootDirective = node->getParent()->getChild("root");
	std::string			root = "./www/";
	if (rootDirective)
	{
		root = rootDirective->getParameter(0);
		if (root.at(0) != '.')
			root = joinPath(".", root);
		rootDirective->setParameter(0, root);
	}

	std::string			errorPagePath = joinPath(joinPath(root, "error_pages"), uri);
	struct stat			st;

	checkPath(errorPagePath, ErrorType::VALIDATOR, "Error page: " + errorPagePath + " not found.", "Error page: " + errorPagePath + " has no reading access.");
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
			
			int new_code = std::stoi(param.substr(1)); // In try-catch block to throw a different error than the one from stoi.
			// std::cout << "DEBUG: New code is: " << new_code << std::endl;
			// Check it's a valid HTTP status code
			if (new_code < 100 || new_code > 599)
				throw ConfigError::validation("Invalid HTTP status code in " + node->getName() + " directive: '" + param + "' must be between 100-599", node);
		}
		else
		{
			// Regular error code
			int error_code = std::stoi(param);
			// Must be a 4xx or 5xx error code
			if (error_code < 400 || error_code > 599)
				throw ConfigError::validation("Invalid error code in " + node->getName() + " directive: '" + param + "' must be between 400-599", node);
		}
	}
	return ;
}
