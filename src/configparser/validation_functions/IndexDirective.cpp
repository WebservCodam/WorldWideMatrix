#include "../Configuration.hpp"

void	validateIndexDirective(Directive* node)
{
    // std::cout << "DEBUG Index Directive: " << std::endl;

	struct stat		st;
	Directive*		rootDirective = node->getParent()->getChild("root");
	std::string		root = "." + rootDirective->getParameter(0);
	std::string		index = node->getParameter(0);
	std::string		indexPath = root + index;	

	if (stat(std::string(indexPath).c_str(), &st) != 0)
		throw ConfigError::validation(std::string("Index path: ") + indexPath + std::string(" doesn't exist."), node);

	if (access(indexPath.c_str(), R_OK) != 0)
		throw ConfigError::validation(std::string("Index path: ") + indexPath + std::string(" cannot be read."), node);
}
