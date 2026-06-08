#include "Configuration.hpp"

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
	// {"main", DirectiveDefinition{"main", true, 1, 10, {"main"}, {"server"}, validateMainDirective}},
	{"user", DirectiveDefinition{"user", false, 0, 2, {"main"}, {}, nullptr}},
	{"server", DirectiveDefinition{"server", true, 0, 0, {"main"}, {"listen", "client_max_body_size", "location"}, validateServerDirective}},

	//	===	Server Context Directives ===
	{"server_name", DirectiveDefinition{"server_name", false, 1, 1, {"server"}, {}, nullptr}},
	{"listen", DirectiveDefinition{"listen", false, 1, 1, {"server"}, {}, validateListenDirective}}, 
	{"keepalive_timeout", DirectiveDefinition{"keepalive_timeout", false, 1, 1, {"server"}, {}, validateKeepaliveTimeoutDirective}},
	{"location", DirectiveDefinition{"location", true, 1, 2, {"server"}, {}, validateLocationDirective}},	// 2 parameters in case it's an equals

	//	=== Location Context Directives ===
	{"root", DirectiveDefinition{"root", false, 1, 1, {"server", "location"}, {}, validateRootDirective}},
	{"index", DirectiveDefinition{"index", false, 1, 1, {"server", "location"}, {}, validateIndexDirective}},
	{"autoindex", DirectiveDefinition{"autoindex", false, 1, 1, {"server", "location"}, {}, validateAutoIndexDirective}},
	{"error_page", DirectiveDefinition{"error_page", false, 2, 100, {"server", "location"}, {}, validateErrorPageDirective}},
	{"client_max_body_size", DirectiveDefinition{"client_max_body_size", false, 1, 1, {"server", "location"}, {}, validateClientMaxBodySizeDirective}},
	{"methods", DirectiveDefinition{"methods", false, 1, 4, {"location"}, {}, validateMethodsDirective}},
	// {"redirect", DirectiveDefinition{"redirect", false, 1, 2, {"location"}, {}, validateRedirectDirective}}, // Check this is valid.
	{"return", DirectiveDefinition{"return", false, 1, 2, {"server", "location"}, {}, validateReturnDirective}},
	{"upload_path", DirectiveDefinition{"upload_path", false, 1, 1, {"location"}, {}, validateUploadPathDirective}},

	// === CGI ===
	{"cgi_handler", DirectiveDefinition{"cgi_handler", false, 2, 2, {"server", "location"}, {}, validateCgiHandlerDirective}},
	// {"cgi_index", DirectiveDefinition{"cgi_index", false, 1, 1, {"server", "location"}, {}, validateCgiIndexDirective}},
};

//	----- GENERAL VALIDATION FUNCTIONS ------

void	validateDirective(Directive* node)
{
	// std::cerr << "DEBUG in validateDirective: " + node->getName() << std::endl; //Provisional.

	// Check that the directive name is valid
	std::map<std::string, DirectiveDefinition>::const_iterator	it = NGINX_DIRECTIVE_SPECS.find(node->getName());
	if (it == NGINX_DIRECTIVE_SPECS.end())
	{
		// std::cerr << "DEBUG in validateDirective 1" << std::endl; //Provisional.
		throw ConfigError::validation("Unknown directive '" + node->getName() + "'", node);
	}

	const DirectiveDefinition&	spec = it->second;

	// Check if context is valid
	if (spec.validContexts.find(node->getContext()) == spec.validContexts.end())
	{
		// std::cerr << "DEBUG in validateDirective 2" << std::endl; //Provisional.
		throw ConfigError::validation("Directive '" + node->getName() + "' is not allowed in '" + node->getContext() + "' context", node);
	}

	// Check parameter count
	if ((spec.minArgs > 0 && node->getParameters().empty())
		|| node->getParameters().size() < spec.minArgs
		|| node->getParameters().size() > spec.maxArgs)
	{
		throw ConfigError::validation("Invalid parameter count for '" + node->getName() + "': expected min:"
									+ std::to_string(spec.minArgs) + " & max: " + std::to_string(spec.maxArgs)
									+ ", got " + std::to_string(node->getParameters().size()), node);
	}

	// Call specific validation function if it exists
	if (spec.validateArgs)
	{
		spec.validateArgs(node);
	}

	return ;
}

// Directives that may legitimately appear more than once in the same block
// (e.g. several `listen` ports or `error_page` entries).
// Every other directive must appear at most once per block.
static const std::set<std::string>	REPEATABLE_DIRECTIVES = {
	"listen", "error_page", "location"
};

// Rejects a once only directive that appears more than once in the same block,
// e.g. two `root` or two `index` in one server/location.
void	checkDuplicateDirectives(Directive* node)
{
	std::set<std::string>	seen;

	for (Directive* child : node->getChildren())
	{
		const std::string&	name = child->getName();

		if (REPEATABLE_DIRECTIVES.find(name) != REPEATABLE_DIRECTIVES.end())
			continue ;
		if (seen.count(name))
			throw ConfigError::validation("Duplicate directive '" + name + "' in '" + node->getName() + "' block", child);
		seen.insert(name);
	}
}

// Rejects two location blocks in the same server that target the same path.
void	checkDuplicateLocations(Directive* node)
{
	std::set<std::string>	seen;

	for (Directive* child : node->getChildren())
	{
		if (child->getName() != "location")
			continue ;

		// A location's identity is its path (the first parameter) — the same
		// value processLocation() keys on.
		const std::string&	path = child->getParameter(0);
		if (seen.count(path))
			throw ConfigError::validation("Duplicate location '" + path + "' in '" + node->getName() + "' block", child);
		seen.insert(path);
	}
}

// Rejects the same HTTP status code mapped by more than one error_page entry,
// e.g. `error_page 404 a.html;` and `error_page 404 b.html;` in one block.
void	checkDuplicateErrorCodes(Directive* node)
{
	std::set<std::string>	seen;

	for (Directive* child : node->getChildren())
	{
		if (child->getName() != "error_page")
			continue ;

		// The last parameter is the target URI; every parameter before it is a
		// status code, except an optional `=code` response-override token.
		const std::vector<std::string>&	params = child->getParameters();
		for (size_t i = 0; i + 1 < params.size(); i++)
		{
			const std::string&	code = params[i];
			if (!code.empty() && code[0] == '=')
				continue ;
			if (seen.count(code))
				throw ConfigError::validation("Duplicate error_page code '" + code + "' in '" + node->getName() + "' block", child);
			seen.insert(code);
		}
	}
}

void	validateBlockDirective(Directive* node)
{
	// Not needed since validateDirective already checks this?
	// if (node->getChildren().empty())
	// 	throw ConfigError::validation("Directive '" + node->getName() + "' expected children directives but didn't have any", node);

	// Validate required children are present
	validateRequiredChildren(node); // Checks presence
	checkDuplicateDirectives(node); // Reject once-only directives that repeat
	checkDuplicateLocations(node);  // Reject two locations with the same path
	checkDuplicateErrorCodes(node); // Reject the same error code mapped twice
	for (Directive* child : node->getChildren())
		validateDirective(child);

	// Validate that each child is in the right context
	validateContext(node);
	return ;
}

void	validateContext(Directive* node)
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
	return ;
}

void	validateRequiredChildren(Directive* node)
{
	// Find the directive specification
	std::map<std::string, DirectiveDefinition>::const_iterator it = NGINX_DIRECTIVE_SPECS.find(node->getName());
	if (it == NGINX_DIRECTIVE_SPECS.end())
		throw ConfigError::validation("Unknown directive '" + node->getName() + "'", node);

	const DirectiveDefinition& spec = it->second;

	// Check each required child directive is present
	for (const std::string& requiredChild : spec.requiredChildren)
	{
		bool found = false;
		for (Directive* child : node->getChildren())
		{
			if (child->getName() == requiredChild)
			{
				found = true;
				break ;
			}
		}
		if (!found)
			throw ConfigError::validation("Directive '" + node->getName() + "' is missing required child directive '" + requiredChild + "'", node);
	}
	return ;
}
