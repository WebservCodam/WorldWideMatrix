#include "../Configuration.hpp"

//	Default root would be the root at the main block.
void	validateRootDirective(Directive* node)
{
	std::string	path = getRoot(node);
	checkPath(path, ErrorType::VALIDATOR, "Root directory: " + path, true);
}
