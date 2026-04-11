#include "../Configuration.hpp"

void	validateCgiHandlerDirective(Directive* node)
{
	std::string	extension = node->getParameter(0);
	std::string	path = node->getParameter(1);
	bool		isValidExtension = false;

	if (extension.empty())
		throw ConfigError::validation("The CGI handler was given an empty extension.", node);
	if (path.empty())
		throw ConfigError::validation("The CGI handler was given an empty path.", node);

	if (extension == CGIExtensions::PYTHON.first)
	{
		isValidExtension = true;
		if (path != CGIExtensions::PYTHON.second)
			throw ConfigError::validation("The CGI handler's path is invalid.", node);
	}
	// else if (extension == CGIExtensions::PHP.first)
	// {
	// 	isValidExtension = true;
	// 	if (path != CGIExtensions::PHP.second)
	// 		throw ConfigError::validation("The CGI handler's path is invalid.", node);
	// }
	if (!isValidExtension)
		throw ConfigError::validation("The CGI handler was given an invalid extension.", node);
}

// void	validateCgiIndexDirective(Directive* node)
// {
// 	// Sets a file name that will be appended after a URI that ends with a slash,
// 	// in the value of the $cgi_script_name variable.
// 	// In other words, it sets a value for the variable $cgi_script_name.
// 	return ;
// }
