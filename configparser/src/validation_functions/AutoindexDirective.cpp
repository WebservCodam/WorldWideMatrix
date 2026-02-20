#include "../../include/Configuration.hpp"

void	validateAutoIndexDirective(Directive* node)
{
	if (node->getParameter(0) == "on" || node->getParameter(0) == "off")
		return ;
	throw ConfigError::validation("Invalid value in " + node->getName() + " directive: '" + node->getParameter(0) + "' (must be 'on' or 'off')", node);
}
