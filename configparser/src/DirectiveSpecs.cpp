#include "../include/Configuration.hpp"

/**
 * The MAP contains the directive name and a 'DirectiveDefinition' object.
 * 
 * A DirectiveDefinition contains:
 * - A std::string name.
 * - A bool that inditcates whether the directive is a block or not.
 * - An int for the minimum amount of arguments for the directive to be valid.
 * - An int for the maximum amount of valid arguments.
 * - A set of std::strings that indicate in what context is the directive valid. I.e. the names of the parent directive (which is a block). If no parent then defaults to 'main'.
 * - A set of std::strings that indicate what sub-directives it requires to be valid. I.e. the names of the necessary children directives.
 * - A pointer to the function that validates the directive.
 */
const std::map<std::string, DirectiveDefinition> NGINX_DIRECTIVE_SPECS =
{
	//	=== Main Context Directives ===
	{"user", DirectiveDefinition{"user", false, 0, 2, {"main"}, {}, nullptr}},

	//	===	Block Directives ===
	{"http", DirectiveDefinition{"http", true, 0, 0, {"main"}, {"server"}, validateHttpDirective}},
	{"server", DirectiveDefinition{"server", true, 0, 0, {"main", "http"}, {"listen", "client_max_body_size", "location"}, validateServerDirective}},
	{"location", DirectiveDefinition{"location", true, 1, 2, {"server", "location"}, {}, validateLocationDirective}},	// 2 parameters in case it's an equals

	// {"allow", DirectiveDefinition{"allow", false, 1, 1, {"http", "server", "location", "limit_except"}, {}, validateAllowOrDenyDirective}},
	// {"deny", DirectiveDefinition{"deny", false, 1, 1, {"http", "server", "location", "limit_except"}, {}, validateAllowOrDenyDirective}},

	//	=== ServerConfig Basics ===
	{"listen", DirectiveDefinition{"listen", false, 1, 1, {"server"}, {}, validateListenDirective}}, 
	{"server_name", DirectiveDefinition{"server_name", false, 1, 100, {"server"}, {}, nullptr}},
	{"root", DirectiveDefinition{"root", false, 1, 1, {"http", "server", "location"}, {}, validateRootDirective}},

	//	=== Autoindex ===
	{"autoindex", DirectiveDefinition{"autoindex", false, 1, 1, {"http", "server", "location"}, {}, validateAutoIndexDirective}},
	{"index", DirectiveDefinition{"index", false, 1, 1, {"http", "server", "location"}, {}, validateIndexDirective}},

	//	=== Error Handling ===
	{"error_page", DirectiveDefinition{"error_page", false, 2, 100, {"http", "server", "location"}, {}, validateErrorPageDirective}},

	// === CGI ===
	// {"fastcgi_pass", DirectiveDefinition{"fastcgi_pass", false, 1, 1, {"location"}, {}, validateFastcgiPassDirective}},
	// {"fastcgi_param", DirectiveDefinition{"fastcgi_param", false, 2, 3, {"http", "server", "location"}, {}, validateFastcgiParamDirective}},
	// {"fastcgi_index", DirectiveDefinition{"fastcgi_index", false, 1, 1, {"http", "server", "location"}, {}, validateFastcgiIndexDirective}},

	//	===	Request Handling ===
	{"return", DirectiveDefinition{"return", false, 1, 2, {"server", "location"}, {}, validateReturnDirective}},	

	//	=== Methods/Limits	===
	{"methods", DirectiveDefinition{"methods", false, 1, 4, {"location"}, {}, validateMethodsDirective}},
	{"client_max_body_size", DirectiveDefinition{"client_max_body_size", false, 1, 1, {"http", "server", "location"}, {}, validateClientMaxBodySizeDirective}}
};

/**
 * MISSING DIRECTIVES
 * 
 * keepalive_timeout
 */

// ----- BLOCK VALIDATION FUNCTIONS -----

bool	validateHttpDirective(Directive* node)
{
	return (validateBlockDirective(node));
}

bool	validateServerDirective(Directive* node)
{
	return (validateBlockDirective(node));
}

bool	validateLocationDirective(Directive* node)
{
	return (validateBlockDirective(node));
}

//	----- GENERAL VALIDATION FUNCTIONS ------

bool	validateDirective(Directive* node)
{
	std::map<std::string, DirectiveDefinition>::const_iterator	it = NGINX_DIRECTIVE_SPECS.find(node->getName());

	if (it == NGINX_DIRECTIVE_SPECS.end())
		throw ConfigError::validation("Unknown directive '" + node->getName() + "'", node);

	const DirectiveDefinition&	spec = it->second;

	// Check if context is valid
	if (spec.validContexts.find(node->getContext()) == spec.validContexts.end())
		throw ConfigError::validation("Directive '" + node->getName() + "' is not allowed in '" + node->getContext() + "' context", node);

	// Check parameter count
	if (node->getParameters().size() < spec.minArgs || node->getParameters().size() > spec.maxArgs)
	{

		std::cout << "DEBUG in validateDirective - The parameters are: " << std::endl;

		for (int i = 0; i < node->getParameters().size(); i++)
		{
			std::cout << "DEBUG: " << node->getParameter(i) << std::endl;
		}

		throw ConfigError::validation("Invalid parameter count for '" + node->getName() + "': expected min:"
									+ std::to_string(spec.minArgs) + " & max: " + std::to_string(spec.maxArgs)
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

bool	validateBlockDirective(Directive* node)
{
	if (node->getChildren().empty())
		throw ConfigError::validation("Directive '" + node->getName() + "' expected children directives but didn't have any", node);

	// Validate required children are present
	if (!validateRequiredChildren(node))
		throw ConfigError::validation("Directive '" + node->getName() + "' didn't contain the necessary children directives", node);

	// Validate that each child is in the right context
	if (!validateContext(node))
		throw ConfigError::validation("Directive '" + node->getName() + "' is in an invalid context", node);

	return (true);
}

bool	validateContext(Directive* node)
{
	for (Directive* currentChild : node->getChildren())
	{
		// Look up the child directive in specs
		std::map<std::string, DirectiveDefinition>::const_iterator it =
			NGINX_DIRECTIVE_SPECS.find(currentChild->getName());

		if (it == NGINX_DIRECTIVE_SPECS.end())
			throw ConfigError::validation("Unknown directive '" + currentChild->getName() + "'", currentChild);

		const DirectiveDefinition&	directiveSpec = it->second;
		bool						validContext = false;

		// Check if current context is valid for this directive
		for (const std::string& context : directiveSpec.validContexts)
		{
			if (currentChild->getContext() == context)
			{
				validContext = true;
				break ;
			}
		}
		if (!validContext)
			throw ConfigError::validation("Directive '" + currentChild->getName() + "' is not allowed in '" + currentChild->getContext() + "' context", currentChild);
	}
	return (true);
}

bool	validateRequiredChildren(Directive* node)
{
	// Find the directive specification
	std::map<std::string, DirectiveDefinition>::const_iterator it = NGINX_DIRECTIVE_SPECS.find(node->getName());
	if (it == NGINX_DIRECTIVE_SPECS.end())
		throw ConfigError::validation("Unknown directive '" + node->getName() + "'", node);

	const DirectiveDefinition& spec = it->second;

	// Check each required child directive
	for (const std::string& requiredChild : spec.requiredChildren)
	{
		bool found = false;
		bool valid = false;
		for (Directive* child : node->getChildren())
		{
			if (child->getName() == requiredChild)
			{
				found = true;
				valid = validateDirective(child);
				break;
			}

		}
		if (!found)
			throw ConfigError::validation("Directive '" + node->getName() + "' is missing required child directive '" + requiredChild + "'", node);

		if (!valid)
		{
			std::cerr << "Directive '" + node->getName() + "' is failing validation." << std::endl;
			return (false);
		}
	}
	return (true);
}
