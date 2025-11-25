#include "../../include/Validator.hpp"
#include "../../include/DirectiveSpecs.hpp"
#include "../../include/ConfigError.hpp"

Validator::Validator(const ConfigFile* configFile) : _ConfigFile(configFile), _directiveSpecs(NGINX_DIRECTIVE_SPECS)
{
	
}

bool	Validator::validate()
{
	const std::vector<std::unique_ptr<Directive>>&	directives = _ConfigFile->getDirectives();

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
		throw ConfigError::validation("Unknown directive '" + node->getName() + "'", node);

	const DirectiveDefinition&	spec = it->second;

	// Check if context is valid
	if (spec.validContexts.find(node->getContext()) == spec.validContexts.end())
		throw ConfigError::validation("Directive '" + node->getName() + "' is not allowed in '" + node->getContext() + "' context", node);

	// Check parameter count
	if (node->getParameters().size() < spec.minArgs || node->getParameters().size() > spec.maxArgs)
	{
		throw ConfigError::validation("Invalid parameter count for '" + node->getName() + "': expected "
									+ std::to_string(spec.minArgs) + "-" + std::to_string(spec.maxArgs)
									+ ", got " + std::to_string(node->getParameters().size()), node);
		// return (false);
	}
	// Call specific validation function if it exists
	if (spec.validateArgs && !spec.validateArgs(node))
	{
		throw ConfigError::validation("Invalid parameter value(s) for '" + node->getName() + "'", node);
		// return (false);
	}

	// If no specific validation, basic checks passed
	return (true);
}
