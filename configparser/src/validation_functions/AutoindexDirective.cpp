#include "../../include/Configuration.hpp"

bool	validateAutoIndexDirective(Directive* node)
{
	if (node->getParameters().empty())
		throw ConfigError::validation("Directive " + node->getName() + " requires a parameter", node);
	if (node->getParameter(0) == "on" || node->getParameter(0) == "off")
		return (true);
	throw ConfigError::validation("Invalid value in " + node->getName() + " directive: '" + node->getParameter(0) + "' (must be 'on' or 'off')", node);
}
