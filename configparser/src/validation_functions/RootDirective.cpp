#include "../../include/Configuration.hpp"

//	Default root would be the root at the main block. (To do after validation)
bool	validateRootDirective(Directive* node)
{
	// If quoted it's because it may contain a space. But same rules apply:
	// If a variable is inside quotes it doesn't expand, otherwise it does.
	// If it is only variable, it has to resolve to an actual path.
	// A path must begin with a '/'

	const std::string&	path = node->getParameter(0);

	// Case 1: Path starts with '/'
	if (path[0] == '/') // Good start
		return (true) ;

	// Case 2: Path starts with '"' (quoted path, may contain spaces)
	else if (path[0] == '"')
	{
		// Extract the content between quotes
		if (path.length() < 3 || path.back() != '"')
			throw ConfigError::validation("Invalid quoted path in '" + node->getName() + "' directive: path must be properly quoted", node);

		std::string	quoted_content = path.substr(1, path.length() - 2);

		// Must still start with '/' after removing quotes
		if (quoted_content.empty() || quoted_content[0] != '/')
			throw ConfigError::validation("Path in '" + node->getName() + "' directive must be an absolute path starting with '/'", node);

		return (true);
	}

	// Would need to verify that it's a valid path.

	// Invalid: doesn't start with '/', '"', or '$'
	throw ConfigError::validation("Invalid path in " + node->getName() + " directive: '" + path + "' must be an absolute path starting with '/'", node);
}
