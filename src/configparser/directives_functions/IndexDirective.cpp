#include "../Configuration.hpp"

void	validateIndexDirective(Directive* node)
{
    // std::cout << "DEBUG Index Directive: " << std::endl;

	Directive*		locationDirective;
	std::string		root;
	std::string		index;
	std::string		indexPath;

	root = getRoot(node);
	locationDirective = node->getParent();
	if (locationDirective->getName() == "location")
	{
		const std::string&	locationPath = locationDirective->getParameter(0);
		root = joinPath(root, locationPath);
	}

	index = node->getParameter(0);
	indexPath = joinPath(root, index);
	checkPath(indexPath, ErrorType::VALIDATOR, "Index path: " + indexPath, false);
}
