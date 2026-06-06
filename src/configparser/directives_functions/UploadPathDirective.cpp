#include "../Configuration.hpp"

void	validateUploadPathDirective(Directive* node)
{
	const std::string&	uploadPath = node->getParameter(0);
	std::string			path = joinPath(".", uploadPath);
	checkPath(path, ErrorType::VALIDATOR, "Upload path: " + path, true);
}
