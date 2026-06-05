#include "../Configuration.hpp"

void	validateUploadPathDirective(Directive* node)
{
	std::string	path = joinPath(".", node->getParameter(0));
	checkPath(path, ErrorType::VALIDATOR, "Upload path: " + path, true);
}
