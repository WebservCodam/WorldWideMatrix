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

bool	validateListenDirective(const Directive* node)
{

	std::cout << "DEBUG: In validateListenDirective" << std::endl;

	std::pair<std::string, std::string>	addressAndPort;	//127.0.0.0:8080?
	bool								isValidAddress;
	bool								isValidPort;

	if (node->getParameters().empty())
		throw ConfigError::validation("Directive '" + node->getName() + "' requires at least one parameter", node);

	addressAndPort = parseAddressAndPort(node->getParameter(0));
	if (addressAndPort.first.empty() && addressAndPort.second.empty())
	{
		if (validateAddress(node->getParameter(0)))
			return (true);
		if (validatePort(node->getParameter(0)))
			return (true);
		throw ConfigError::validation("Invalid address or port format in '" + node->getName() + "' directive: '" + node->getParameter(0) + "'", node);
	}
	isValidAddress = validateAddress(addressAndPort.first);
	isValidPort = validatePort(addressAndPort.second);
	if (isValidAddress && addressAndPort.second.empty())
		return (true);
	else if (isValidAddress && isValidPort)
		return (true);
	else if (addressAndPort.first.empty() && isValidPort)
		return (true);
	throw ConfigError::validation("Invalid address:port combination in '" + node->getName() + "' directive: '" + node->getParameter(0) + "'", node);
}

std::pair<std::string, std::string>	parseAddressAndPort(const std::string& address)
{
	std::pair<std::string, std::string>	addressAndPort = {"", ""};
	std::string	addressPart;
	std::string	portPart;
	size_t		pos;

	if (address.empty())
		return (addressAndPort);
	pos = address.find(":");
	if (pos == address.npos)
		return (addressAndPort);
	else
	{
		addressPart = address.substr(0, pos);
		portPart = address.substr(pos + 1, address.size() - (pos + 1));
		addressAndPort = {std::move(addressPart), std::move(portPart)};
	}

	std::cout << "DEBUG in parserAddressAndPort - Address: " << addressAndPort.first << " Port: " << addressAndPort.second << std::endl;

	return (addressAndPort);
}

bool	validateAddress(const std::string& address)
{
	size_t		currentPos = 0;
	size_t		nextPos;
	size_t		iterations = 0;
	std::string	currentChunk;

	if (address.empty())
		return (false);

	if (address == "localhost")
		return (true);

	while (currentPos < address.length())
	{
		nextPos = address.find(".", currentPos);
		if (nextPos == std::string::npos)
			currentChunk = address.substr(currentPos);
		else
			currentChunk = address.substr(currentPos, nextPos - currentPos);

		if (!isByte(currentChunk))
			return (false);

		iterations++;
		if (nextPos == std::string::npos)
			break;
		currentPos = nextPos + 1;
	}
	if (iterations != 4)
		return (false);
	return (true);
}

bool	validatePort(const std::string& port)
{
	if (port.empty())
		return (false);
	try
	{
		int portNumber = std::stoi(port);
		if (port.size() != std::to_string(portNumber).length())
			throw (std::runtime_error("Wrong port"));
		if (portNumber >= 1 && portNumber <= 65535)
			return (true);
	}
	catch(const std::exception& e)
	{
		return (false);
	}
	return (false);
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

bool	validateReturnDirective(const Directive* node)
{
	// return 301 http://example.com;
	// return 404;
	// return 200 "some text";

	if (node->getParameters().empty() || node->getParameters().size() > 2)
		throw ConfigError::validation("Directive " + node->getName() + " requires 1 or 2 parameters", node);

	// First parameter must be a valid HTTP status code
	try
	{
		int status_code = std::stoi(node->getParameter(0));

		// Must be a valid HTTP status code (100-599)
		if (status_code < 100 || status_code > 599)
			throw ConfigError::validation("Invalid HTTP status code in " + node->getName() + " directive: '" + node->getParameter(0) + "' must be between 100-599", node);
	}
	catch (const ConfigError&)
	{
		throw;
	}
	catch (const std::exception&)
	{
		throw ConfigError::validation("Invalid HTTP status code in " + node->getName() + " directive: '" + node->getParameter(0) + "'", node);
	}

	// If there's a second parameter, validate it as URL or text
	if (node->getParameters().size() == 2)
	{
		const std::string& second_param = node->getParameter(1);

		if (second_param.empty())
			throw ConfigError::validation("Second parameter in " + node->getName() + " directive cannot be empty", node);

		// For URLs starting with http:// or https://
		if (second_param.find("http://") == 0 || second_param.find("https://") == 0)
		{
			// Basic URL validation - must have something after protocol
			if (second_param.length() <= 8) // "https://" is 8 chars
				throw ConfigError::validation("Invalid URL in " + node->getName() + " directive: '" + second_param + "'", node);
		}
		// For quoted text
		else if (second_param.front() == '"' && second_param.back() == '"')
		{
			// Must have content between quotes
			if (second_param.length() < 3)
				throw ConfigError::validation("Invalid quoted text in " + node->getName() + " directive: must have content between quotes", node);
		}
		// For relative URLs or paths
		else if (second_param.front() == '/')
		{
			// Check for valid path characters and format
			for (size_t i = 0; i < second_param.length(); ++i)
			{
				char c = second_param[i];
				// Allow alphanumeric, slash, dash, underscore, dot, percent (for URL encoding)
				if (!std::isalnum(c) && c != '/' && c != '-' && c != '_' && c != '.' && c != '%')
					throw ConfigError::validation("Invalid path character in " + node->getName() + " directive: '" + second_param + "'", node);
			}

			// Check for consecutive slashes (except at start)
			if (second_param.find("//") != std::string::npos)
				throw ConfigError::validation("Invalid path in " + node->getName() + "  directive: '" + second_param + "' contains consecutive slashes", node);
		}
		else
		{
			// Other text content is also valid
		}
	}

	return (true);
}

bool	validateAllowMethodsDirective(const Directive* node)
{
	const std::vector<std::string> httpMethods = {"GET", "POST", "DELETE", "HEAD"};

	if (node->getParameters().empty() || node->getChildren().empty())
		throw ConfigError::validation("Directive " + node->getName() + "  requires at least one HTTP method parameter and child directives", node);
	return (true);
}

// bool	validateLimitExceptDirective(const Directive* node)
// {
// 	// It should take a method as a parameter.
// 	// If GET is allowed, so is HEAD.
// 	// It allows these methods inside a location.
// 	// Within, it should have directives 'allow' and/or 'deny'.

// 	const std::vector<std::string> httpMethods = {"GET", "POST", "DELETE", "HEAD"};

// 	if (node->getParameters().empty() || node->getChildren().empty())
// 		throw ConfigError::validation("Directive 'limit_except' requires at least one HTTP method parameter and child directives", node);

// 	for (const std::string& currentMethod : node->getParameters())
// 	{
// 		bool	isValidMethod = false;
// 		for (const std::string& allowed : httpMethods)
// 		{
// 			if (currentMethod == allowed)
// 			{
// 				isValidMethod = true;
// 				break ;
// 			}
// 		}
// 		if (isValidMethod == false)
// 			throw ConfigError::validation("Invalid HTTP method in 'limit_except' directive: '" + currentMethod + "' (must be GET, POST, DELETE, or HEAD)", node);
// 	}
// 	//	Validate children
// 	for (const Directive*	currentChild : node->getChildren())
// 	{
// 		if (currentChild->getName() != "allow" && currentChild->getName() != "deny")
// 			throw ConfigError::validation("Directive 'limit_except' can only contain 'allow' or 'deny' directives, found: '" + currentChild->getName() + "'", node);
// 		if (!validateAllowOrDeny(currentChild))
// 			throw ConfigError::validation("Invalid 'allow' or 'deny' directive inside 'limit_except'", node);
// 	}
// 	return (true);
// }

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
