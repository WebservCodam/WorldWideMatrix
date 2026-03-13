#include "../Configuration.hpp"

void	validateKeepaliveTimeoutDirective(Directive* node)
{
	const std::string&	param = node->getParameter(0);
	int					timeout;

	if (param.empty())
		throw ConfigError::validation("Invalid size value in '" + node->getName() + "' directive", node);

	// Convert parameter to int.
	timeout = std::stoi(param);

	// Check if the string is purely a number.
	int	num_len = std::to_string(timeout).length();
	if (num_len != param.length())
		throw ConfigError::validation("Directive '" + node->getName() + "' is not a number.", node);
	
	if (timeout > 0)
		return ;
	else
		throw ConfigError::validation("Directive '" + node->getName() + "' has an invalid keep-alive timer.\n"
									+ "The timeout should be greater than 0. The parameter value is: "
									+ node->getParameter(0), node);
	return ;
}

/**
 * @brief
 * 
 * @return
 */
int	ConfigFile::processKeepaliveTimeout(Directive* directive)
{
	if (directive)
		return (std::stoi(directive->getParameter(0)));
	else
		return (DEFAULT_KEEP_ALIVE_TIMEOUT);
}
