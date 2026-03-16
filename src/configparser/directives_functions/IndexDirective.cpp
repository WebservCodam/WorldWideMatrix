#include "../Configuration.hpp"

void	validateIndexDirective(Directive* node)
{
    // std::cout << "DEBUG Index Directive: " << std::endl;

	Directive*		rootDirective;
	std::string		root;
	std::string		index;
	std::string		indexPath;

	rootDirective = node->getParent()->getChild("root");
	if (!rootDirective && node->getParent()->getParent())
		rootDirective = node->getParent()->getParent()->getChild("root");
	if (!rootDirective)
		throw ConfigError::validation(std::string("Root directive couldn't be found."), node);

	root = rootDirective->getParameter(0);
	if (root.at(0) != '.')
		root = joinPath(".", root);
	rootDirective->setParameter(0, root);

	index = node->getParameter(0);
	indexPath = root + index;
	
	checkPath(indexPath, ErrorType::VALIDATOR, "Index path", false);
}
