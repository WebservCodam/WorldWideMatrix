#include "../Configuration.hpp"

void	validateServerDirective(Directive* node)
{
	// std::string	path = getRoot(node);
	validateBlockDirective(node);
}

/**
 * @return
 * If there's a server name directive, it returns the given server name.
 * Otherwise it returns 'unnamed_server'.
 */
std::string	ConfigFile::processServerName(const Directive* directive)
{
	if (directive)
		return (directive->getParameter(0));
	return ("unnamed_server");
}
