#include "../Configuration.hpp"

// Any extension is accepted; the only requirement is that the configured
// interpreter exists and is executable. A request whose file matches this
// extension runs through this interpreter; an executable file with no
// matching handler is run on its own (see Cgi::buildArgv).
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
	if (access(path.c_str(), X_OK) != 0)
		throw ConfigError::validation("The CGI interpreter '" + path + "' is not an executable file.", node);
}
