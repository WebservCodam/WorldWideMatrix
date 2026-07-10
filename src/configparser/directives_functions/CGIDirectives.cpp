#include "../Configuration.hpp"

void	validateCgiHandlerDirective(Directive* node)
{
	std::string	extension = node->getParameter(0);
	std::string	path = node->getParameter(1);

	if (extension.empty())
		throw ConfigError::validation("The CGI handler was given an empty extension.", node);
	if (extension[0] != '.')
		throw ConfigError::validation("The CGI handler extension must start with a '.'.", node);
	if (path.empty())
		throw ConfigError::validation("The CGI handler was given an empty interpreter path.", node);

	// Any extension is accepted; the only requirement is that the configured
	// interpreter exists and is executable. A request whose file matches this
	// extension runs through this interpreter; an executable file with no
	// matching handler is run on its own (see Cgi::buildArgv).
	if (access(path.c_str(), X_OK) != 0)
		throw ConfigError::validation("The CGI interpreter '" + path + "' is not an executable file.", node);
}

// void	validateCgiIndexDirective(Directive* node)
// {
// 	// Sets a file name that will be appended after a URI that ends with a slash,
// 	// in the value of the $cgi_script_name variable.
// 	// In other words, it sets a value for the variable $cgi_script_name.
// 	return ;
// }
