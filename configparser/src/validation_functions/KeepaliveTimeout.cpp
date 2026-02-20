#include "../../include/Configuration.hpp"

void	validateKeepaliveTimeoutDirective(Directive* node)
{
	if (node->getParameters().empty())
		throw ConfigError::validation("Directive '" + node->getName() + "' requires a parameter", node);	// I belive this is checked in requiredArguments, so it should never enter this condition.
	
	try
	{
		const std::string&	param = node->getParameter(0);
		int					timeout;

		if (param.empty())
			throw ConfigError::validation("Invalid size value in '" + node->getName() + "' directive", node);

		// Convert parameter to int.
		try
		{
			timeout = std::stoi(param);
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}

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
	}
	catch(const ConfigError&)
	{
		throw ; // Rethrows the config error, outside of the function now.
	}
	catch(const std::exception& e)
	{
		throw ConfigError::validation("Invalid size format in '" + node->getName() + "' directive: '" + node->getParameter(0) + "'", node);
	}
	return ;
}
