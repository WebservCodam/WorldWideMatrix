#include "../Configuration.hpp"

void	validateIndexDirective(Directive* node)
{
    std::cout << "DEBUG Index Directive: " << std::endl;
	// std::cout << node->get // I need to include the parent in each Directive...
	if (node->getParameter(0) == "index.html") //Hardcoded
		return ;
	throw ConfigError::validation("Invalid index file in " + node->getName() + " directive: '" + node->getParameter(0) + "' (only 'index.html' is currently supported)", node);
}
