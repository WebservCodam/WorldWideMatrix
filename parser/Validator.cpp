#include "../include/Validator.hpp"
#include "../include/DirectiveSpecs.hpp"
#include "../include/ParseError.hpp"

Validator::Validator(std::unique_ptr<ConfigFile>& configFile) : _ConfigFile(std::move(configFile)), _directiveSpecs(NGINX_DIRECTIVE_SPECS)
{
	
}
Validator::~Validator()
{

}

bool	Validator::validate()
{
	return (false);
}

bool	Validator::validateDirective(const Directive* node)
{
	std::map<std::string, DirectiveDefinition>::iterator	it = _directiveSpecs.find(node->name);
	if (it == _directiveSpecs.end())
		return (false);

	const DirectiveDefinition&	spec = it->second;

	// Check if context is valid
	if (spec.validContexts.find(node->context) == spec.validContexts.end())
		return (false);

	// Check parameter count
	if (node->parameters.size() < spec.minArgs || node->parameters.size() > spec.maxArgs)
		return (false);

	// Call specific validation function if it exists
	if (spec.validateArgs)
		return spec.validateArgs(node);

	// If no specific validation, basic checks passed
	return (true);
}

bool	Validator::validateUserDirective(const Directive* node)
{
	return validateDirective(node);
}

// Validation functions for specific directives
bool validateWorkerProcessesArgs(const Directive* node)
{
	if (node->parameters.at(0) == "auto")
		return (true);

	try
	{
		int processes = std::stoi(node->parameters.at(0));
		return (processes > 0 && processes <= 16);
	}
	catch (const std::exception& e)
	{
		return (false);
	}
}

/**
 * If the number of worker processes is set to auto, then it's valid
 * otherwise the allowed number of worker processes must be between 1 and 16 (inclusive).
 */
bool	Validator::validateWorkerProcessesDirective(const Directive* node)
{
	if (node->name == "worker_processes")
	{
		if (node->parameters.at(0) == "auto")
			return (true);
		else
		{
			try
			{
				int processes = std::stoi(node->parameters.at(0));

				if (processes > 0 && processes <= 16)
					return (true);
			}
			catch (const ParseError& e)
			{
				throw ;
			}
		}
	}
	return (false);
}
bool	Validator::validateHttpDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateServerDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateLocationDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateListenDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateServerNameDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateRootDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateIndexDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateErrorPageDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateFastcgiPassDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateFastcgiParamDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateFastcgiIndexDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateReturnDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateRewriteDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateLimitExceptDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateClientBodyTempPathDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateClientMaxBodySizeDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateAutoIndexDirective(const Directive* node)
{
	return (false);
}
