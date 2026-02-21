#include "../../include/Configuration.hpp"

//	Default root would be the root at the main block. (To do after validation)
//	To do: create function that prepends the root to the location given by the request.
void	validateRootDirective(Directive* node)
{
	// This currently assumes that directories are listed as relative paths.

	const std::string&	path = std::string("./") + node->getParameter(0);
	struct stat			st;

	if (stat(path.c_str(), &st) != 0)
		throw ConfigError::validation(std::string("Root directory: ") + path + std::string(" doesn't exist"), node);
	
	if (!S_ISDIR(st.st_mode))
		throw ConfigError::validation(std::string("Root directory: ") + path + std::string(" is not a directory"), node);

	if (access(path.c_str(), R_OK | X_OK) != 0)
        throw ConfigError::validation(std::string("Root directory: ") + path + std::string(" doesn't have reading or executing permissions"), node);

	return ;
}
