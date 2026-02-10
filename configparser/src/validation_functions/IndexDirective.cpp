#include "../../include/Configuration.hpp"

bool	validateIndexDirective(const Directive* node)
{
	if (node->getParameters().empty())
		throw ConfigError::validation("Directive " + node->getName() + " requires at least one parameter", node);
	if (node->getParameter(0) == "index.html") //Hardcoded
		return (true);
	throw ConfigError::validation("Invalid index file in " + node->getName() + " directive: '" + node->getParameter(0) + "' (only 'index.html' is currently supported)", node);
}
