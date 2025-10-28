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
