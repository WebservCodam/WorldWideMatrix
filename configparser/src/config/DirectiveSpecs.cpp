#include "../../include/Configuration.hpp"

/**
 * The MAP contains the directive name and a DirectiveDefinition object.
 * 
 * A DirectiveDefinition contains:
 * - A name,
 * - Whether the directive is a block or not.
 * - The minimum amount of arguments for the directive to be valid.
 * - The maximum amount of valid arguments.
 * - In what context is the directive valid? I.e. the names of the parent directive (which is a block). If no parent then defaults to 'main'.
 * - What sub-directives does it require to be valid? I.e. the names of the necessary children directives.
 * - A pointer to the function that validates the directive.
 */

const std::map<std::string, DirectiveDefinition> NGINX_DIRECTIVE_SPECS =
{
	//	=== Main Context Directives ===
	{"user", DirectiveDefinition{"user", false, 0, 2, {"main"}, {}, nullptr}},

	//	===	Block Directives ===
	{"http", DirectiveDefinition{"http", true, 0, 0, {"main"}, {"server"}, validateHttpDirective}},
	{"server", DirectiveDefinition{"server", true, 0, 0, {"main", "http"}, {"listen"}, validateServerDirective}},
	{"location", DirectiveDefinition{"location", true, 1, 2, {"server", "location"}, {}, validateLocationDirective}},	// 2 parameters in case it's an equals

	{"allow", DirectiveDefinition{"allow", false, 1, 1, {"http", "server", "location", "limit_except"}, {}, validateAllowOrDenyDirective}},
	{"deny", DirectiveDefinition{"deny", false, 1, 1, {"http", "server", "location", "limit_except"}, {}, validateAllowOrDenyDirective}},

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
	{"allow_methods", DirectiveDefinition{"allow_methods", false, 1, 4, {"location"}, {}, validateAllowMethodsDirective}},
	{"client_max_body_size", DirectiveDefinition{"client_max_body_size", false, 1, 1, {"http", "server", "location"}, {}, validateClientMaxBodySizeDirective}}
};



// ----- SPECIFIC VALIDATION FUNCTIONS -----

bool	validateBlockDirective(const Directive* node)
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

bool	validateHttpDirective(const Directive* node)
{
	return (validateBlockDirective(node));
}

bool	validateServerDirective(const Directive* node)
{
	return (validateBlockDirective(node));
}

bool	validateLocationDirective(const Directive* node)
{
	return (validateBlockDirective(node));
}

bool	validateAllowOrDenyDirective(const Directive* node)
{
	const std::string&	address = node->getParameters().at(0);
	size_t				cidrPos = 0;
	std::string			addressPart;
	std::string			cidrPart;

	if (address.empty())
		throw ConfigError::validation("Directive '" + node->getName() + "' requires a non-empty address parameter", node);

	if (address == "all")
		return (true);

	cidrPos = address.find("/");
	if (cidrPos != std::string::npos)
	{
		addressPart = address.substr(0, cidrPos);
		cidrPart = address.substr(cidrPos + 1);
		try
		{
			int	cidr = std::stoi(cidrPart);
			if (cidrPart.length() != std::to_string(cidr).length())
				throw ConfigError::validation("Invalid CIDR notation in '" + node->getName() + "': '" + address + "'", node);
			if (cidr < 0 || cidr > 32)
				throw ConfigError::validation("CIDR value must be between 0 and 32 in '" + node->getName() + "': '" + address + "'", node);
		}
		catch(const ConfigError&)
		{
			throw;
		}
		catch(const std::exception& e)
		{
			throw ConfigError::validation("Invalid CIDR value in '" + node->getName() + "': '" + address + "'", node);
		}
		if (!validateAddress(addressPart))
			throw ConfigError::validation("Invalid IP address in '" + node->getName() + "': '" + addressPart + "'", node);
	}
	else
	{
		if (!validateAddress(address))
			throw ConfigError::validation("Invalid IP address in '" + node->getName() + "': '" + address + "'", node);
	}
	return (true);
}

//	Default root would be the root at the http block. (To do after validation)
bool	validateRootDirective(const Directive* node)
{
	// If quoted it's because it may contain a space. But same rules apply:
	// If a variable is inside quotes it doesn't expand, otherwise it does.
	// If it is only variable, it has to resolve to an actual path.
	// A path must begin with a '/'

	const std::string&	path = node->getParameter(0);

	// Case 1: Path starts with '/'
	if (path[0] == '/') // Good start
		return (true) ;

	// Case 2: Path starts with '"' (quoted path, may contain spaces)
	else if (path[0] == '"')
	{
		// Extract the content between quotes
		if (path.length() < 3 || path.back() != '"')
			throw ConfigError::validation("Invalid quoted path in '" + node->getName() + "' directive: path must be properly quoted", node);

		std::string	quoted_content = path.substr(1, path.length() - 2);

		// Must still start with '/' after removing quotes
		if (quoted_content.empty() || quoted_content[0] != '/')
			throw ConfigError::validation("Path in '" + node->getName() + "' directive must be an absolute path starting with '/'", node);

		return (true);
	}

	// Would need to verify that it's a valid path.

	// Invalid: doesn't start with '/', '"', or '$'
	throw ConfigError::validation("Invalid path in " + node->getName() + " directive: '" + path + "' must be an absolute path starting with '/'", node);
}

bool	validateIndexDirective(const Directive* node)
{
	if (node->getParameters().empty())
		throw ConfigError::validation("Directive " + node->getName() + " requires at least one parameter", node);
	if (node->getParameter(0) == "index.html") //Hardcoded
		return (true);
	throw ConfigError::validation("Invalid index file in " + node->getName() + " directive: '" + node->getParameter(0) + "' (only 'index.html' is currently supported)", node);
}

bool	validateAutoIndexDirective(const Directive* node)
{
	if (node->getParameters().empty())
		throw ConfigError::validation("Directive " + node->getName() + " requires a parameter", node);
	if (node->getParameter(0) == "on" || node->getParameter(0) == "off")
		return (true);
	throw ConfigError::validation("Invalid value in " + node->getName() + " directive: '" + node->getParameter(0) + "' (must be 'on' or 'off')", node);
}

bool	validateErrorPageDirective(const Directive* node)
{
	// error_page 404 /404.html;			The last value is a URI.
	// error_page 500 502 503 /50x.html;	There can be multiple codes.
	// error_page 404 =200 /empty.gif;		An equals changes the code.

	if (node->getParameters().size() < 2)
		throw ConfigError::validation("Directive " + node->getName() + " requires at least 2 parameters (error code and URI)", node);

	// Last parameter is always the URI
	const std::string& uri = node->getParameters().back();

	// Check URI format (should start with / or be a valid URL)
	if (uri.empty() || (uri[0] != '/' && uri.find("http") != 0))
		throw ConfigError::validation("Invalid URI in " + node->getName() + " directive: '" + uri + "' must start with '/' or 'http'", node);

	// All parameters except the last one are error codes
	for (size_t i = 0; i < node->getParameters().size() - 1; ++i)
	{
		const std::string& param = node->getParameter(i);

		// Check for response code change (e.g., "=200")
		if (param[0] == '=')
		{
			if (param.length() < 2)
				throw ConfigError::validation("Invalid response code change in " + node->getName() + " directive: '" + param + "'", node);
			try
			{
				int new_code = std::stoi(param.substr(1));
				// Validate it's a valid HTTP status code
				if (new_code < 100 || new_code > 599)
					throw ConfigError::validation("Invalid HTTP status code in " + node->getName() + " directive: '" + param + "' must be between 100-599", node);
			}
			catch (const ConfigError&)
			{
				throw;
			}
			catch (const std::exception&)
			{
				throw ConfigError::validation("Invalid response code change in " + node->getName() + " directive: '" + param + "'", node);
			}
		}
		else
		{
			// Regular error code
			try
			{
				int error_code = std::stoi(param);
				// Must be a 4xx or 5xx error code
				if (error_code < 400 || error_code > 599)
					throw ConfigError::validation("Invalid error code in " + node->getName() + " directive: '" + param + "' must be between 400-599", node);
			}
			catch (const ConfigError&)
			{
				throw;
			}
			catch (const std::exception&)
			{
				throw ConfigError::validation("Invalid error code in " + node->getName() + " directive: '" + param + "'", node);
			}
		}
	}
	return (true);
}

bool	validateFastcgiPassDirective(const Directive* node)
{
	// Sets the address for a FastCGI server.
	// The address can be specified as a domain name or IP address, and a port (e.g.): localhost:9000;
	// If a domain name resolves to several addresses, all of them will be used in a round-robin fashion. But we don't need that...
	return (false);
}

bool	validateFastcgiParamDirective(const Directive* node)
{
	// Sets a parameter that should be passed to the FastCGI server.
	// The value can contain text, variables, and their combination.

	// If the directive is specified with if_not_empty
	// then such a parameter will be passed to the server only if its value is not empty.
	// We can skip that.

	return (false);
}

bool	validateFastcgiIndexDirective(const Directive* node)
{
	// Sets a file name that will be appended after a URI that ends with a slash,
	// in the value of the $fastcgi_script_name variable.
	// In other words, it sets a value for the variable $fastcgi_script_name.
	return (false);
}

bool	validateAllowMethodsDirective(const Directive* node)
{
	const std::vector<std::string> httpMethods = {"GET", "POST", "DELETE", "HEAD"};

	if (node->getParameters().empty() || node->getChildren().empty())
		throw ConfigError::validation("Directive " + node->getName() + "  requires at least one HTTP method parameter and child directives", node);
	return (true);
}

bool	validateClientMaxBodySizeDirective(const Directive* node)
{
	// If 0 == no limit
	// Can be 1m or 10m which means 10 megabytes. Any request that exceeds that it returns a 413 error.
	// The letter can be lower or upper case.
	// It can also be a number

	if (node->getParameters().empty())
		throw ConfigError::validation("Directive '" + node->getName() + "' requires a parameter", node);
	if (node->getParameter(0) == "0")
		return (true);
	try
	{
		const std::string& param = node->getParameter(0);
		if (param.empty())
			throw ConfigError::validation("Invalid size value in '" + node->getName() + "' directive", node);

		char lastChar = param.back(); // IT CAN ALSO BE JUST A NUMBER!
		if (lastChar != 'm' && lastChar != 'M')
			throw ConfigError::validation("Invalid size unit in '" + node->getName() + "' directive: '" + param + "' (must end with 'm' or 'M')", node);

		std::string numPart = param.substr(0, param.length() - 1);
		int	size = std::stoi(numPart);
		if (size < 1 || size > 10)
			throw ConfigError::validation("Invalid size value in '" + node->getName() + "' directive: '" + param + "' (must be between 1m and 10m)", node);
	}
	catch(const ConfigError&)
	{
		throw ; // Rethrows the config error, outside of the function now.
	}
	catch(const std::exception& e)
	{
		throw ConfigError::validation("Invalid size format in '" + node->getName() + "' directive: '" + node->getParameter(0) + "'", node);
	}
	return (true);
}


//	----- UTILITIES ------

bool	validateDirective(const Directive* node)
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

bool	validateContext(const Directive* node)
{
	for (const Directive* currentChild : node->getChildren())
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

bool	validateRequiredChildren(const Directive* node)
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
		for (const Directive* child : node->getChildren())
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

bool isByte(std::string &number)
{
	if (number.empty())
		return (false);

    try
	{
		int	num = std::stoi(number);
		if (number.size() != std::to_string(num).length())
			return (false);
		if (num > 255 || num < 0)
			return (false);
	}
	catch(const std::exception& e)
	{
		return (false);
	}	
    return (true);
}
