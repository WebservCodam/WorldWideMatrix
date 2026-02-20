#include "../../include/Configuration.hpp"

void	validateMethodsDirective(Directive* node)
{
	const std::vector<std::string> httpMethods = {"GET", "POST", "DELETE", "HEAD"};

	// if (node->getParameters().empty() || node->getChildren().empty())
	// 	throw ConfigError::validation("Directive " + node->getName() + "  requires at least one HTTP method parameter and child directives", node);

	// COMPLETE 

	return ;
}
