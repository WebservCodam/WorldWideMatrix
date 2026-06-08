#include "../Configuration.hpp"

void	validateAutoIndexDirective(Directive* node)
{
	const std::string&	toggle = node->getParameter(0);

	if (toggle == "on" || toggle == "off")
		return ;
	throw ConfigError::validation("Invalid value in " + node->getName() + " directive: '" + toggle + "' (must be 'on' or 'off')", node);
}
