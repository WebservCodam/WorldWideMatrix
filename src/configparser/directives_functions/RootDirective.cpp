#include "../Configuration.hpp"

//	Default root would be the root at the main block. (To do after validation)
//	To do: create function that prepends the root to the location given by the request.
void	validateRootDirective(Directive* node)
{
	std::string	path = getRoot(node);
	checkPath(path, ErrorType::VALIDATOR, "Root directory: " + path, true);
}
