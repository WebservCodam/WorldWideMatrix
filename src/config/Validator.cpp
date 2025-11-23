#include "../../include/Validator.hpp"
#include "../../include/DirectiveSpecs.hpp"

Validator::Validator(std::unique_ptr<ConfigFile>& configFile) : _ConfigFile(std::move(configFile)), _directiveSpecs(NGINX_DIRECTIVE_SPECS)
{
	
}

bool	Validator::validate()
{
	const std::vector<std::unique_ptr<Directive>>&	directives = _ConfigFile.get()->getDirectives();

	for (const std::unique_ptr<Directive>& directive : directives)
	{
		if (!validateDirective(directive.get()))
			return (false);
	}
	return (true);
}

bool	Validator::validateDirective(const Directive* node)
{
	std::map<std::string, DirectiveDefinition>::iterator	it = _directiveSpecs.find(node->getName());
	if (it == _directiveSpecs.end())
	{
		throw (std::runtime_error("Couldn't find " + node->getName() + " in the list of directive definitions."));
		// return (false);
	}

	const DirectiveDefinition&	spec = it->second;

	// Check if context is valid
	if (spec.validContexts.find(node->getContext()) == spec.validContexts.end())
	{
		throw (std::runtime_error("Invalid context for " + node->getName() + "."));
		// return (false);
	}

	// Check parameter count
	if (node->getParameters().size() < spec.minArgs || node->getParameters().size() > spec.maxArgs)
	{
		throw (std::runtime_error("Invalid parameter count for " + node->getName() + "."));
		// return (false);
	}
	// Call specific validation function if it exists
	if (spec.validateArgs && !spec.validateArgs(node))
	{
		throw (std::runtime_error(node->getName() + " didn't pass validation for the specifics of this directive."));
		// return (false);
	}

	// If no specific validation, basic checks passed
	return (true);
}
