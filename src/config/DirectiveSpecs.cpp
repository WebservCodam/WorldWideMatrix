#include "../../include/DirectiveSpecs.hpp"

const std::map<std::string, DirectiveDefinition> NGINX_DIRECTIVE_SPECS =
{
	//	=== Main Context Directives ===
	{"user", DirectiveDefinition{"user", false, 0, 2, {"main"}, {}, nullptr}},
	{"worker_processes", DirectiveDefinition{"worker_processes", false, 1, 1, {"main"}, {}, validateWorkerProcessesDirective}},

	//	===	Block Directives ===
	{"http", DirectiveDefinition{"http", true, 0, 0, {"main"}, {"server"}, validateHttpDirective}},
	{"server", DirectiveDefinition{"server", true, 0, 0, {"http"}, {"listen"}, validateServerDirective}},
	{"location", DirectiveDefinition{"location", true, 1, 2, {"server", "location"}, {}, validateLocationDirective}},	// 2 parameters in case it's an equals

	{"allow", DirectiveDefinition{"allow", false, 1, 1, {"http", "server", "location", "limit_except"}, {}, validateAllowOrDeny}},
	{"deny", DirectiveDefinition{"deny", false, 1, 1, {"http", "server", "location", "limit_except"}, {}, validateAllowOrDeny}},

	//	=== Server Basics ===
	{"listen", DirectiveDefinition{"listen", false, 1, 1, {"server"}, {}, validateListenDirective}}, 
	{"server_name", DirectiveDefinition{"server_name", false, 1, 100, {"server"}, {}, nullptr}},
	{"root", DirectiveDefinition{"root", false, 1, 1, {"http", "server", "location"}, {}, validateRootDirective}},

	//	=== Autoindex ===
	{"autoindex", DirectiveDefinition{"autoindex", false, 1, 1, {"http", "server", "location"}, {}, validateAutoIndexDirective}},
	{"index", DirectiveDefinition{"index", false, 1, 1, {"http", "server", "location"}, {}, validateIndexDirective}},

	//	=== Error Handling ===
	{"error_page", DirectiveDefinition{"error_page", false, 2, 100, {"http", "server", "location"}, {}, validateErrorPageDirective}},

	// === CGI ===
	{"fastcgi_pass", DirectiveDefinition{"fastcgi_pass", false, 1, 1, {"location"}, {}, validateFastcgiPassDirective}},
	{"fastcgi_param", DirectiveDefinition{"fastcgi_param", false, 2, 3, {"http", "server", "location"}, {}, validateFastcgiParamDirective}},
	{"fastcgi_index", DirectiveDefinition{"fastcgi_index", false, 1, 1, {"http", "server", "location"}, {}, validateFastcgiIndexDirective}},

	//	===	Request Handling ===
	{"return", DirectiveDefinition{"return", false, 1, 2, {"server", "location"}, {}, validateReturnDirective}},	

	//	=== Methods/Limits	===
	{"limit_except", DirectiveDefinition{"limit_except", true, 1, 10, {"location"}, {}, validateLimitExceptDirective}},	
	{"client_max_body_size", DirectiveDefinition{"client_max_body_size", false, 1, 1, {"http", "server", "location"}, {}, validateClientMaxBodySizeDirective}}
};


bool	validateAllowOrDeny(const Directive* node)
{
	const std::string&	address = node->getParameters().at(0);
	size_t				cidrPos = 0;
	std::string			addressPart;
	std::string			cidrPart;

	if (address.empty())
		return (false);

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
				return (false);
			if (cidr < 0 || cidr > 32)
				return (false);
		}
		catch(const std::exception& e)
		{
			return (false);
		}
		if (!validateAddress(addressPart))
			return (false);
	}
	else
		return (validateAddress(address));
	return (true);
}

// Validation functions for specific directives

//Why would we configure this?
bool	validateWorkerProcessesDirective(const Directive* node)
{
	if (node->getParameter(0) == "auto")
		return (true);
	try
	{
		int processes = std::stoi(node->getParameter(0));
		return (processes > 0 && processes <= 16);
	}
	catch (const std::exception& e)
	{
		return (false);
	}
}

bool	validateContext(const Directive* node)
{
	for (const Directive* currentChild : node->getChildren())
	{
		// Look up the child directive in specs
		std::map<std::string, DirectiveDefinition>::const_iterator it = 
			NGINX_DIRECTIVE_SPECS.find(currentChild->getName());

		if (it == NGINX_DIRECTIVE_SPECS.end())
			return (false); // Unknown directive
		
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
			return (false); // Throw invalid context.
	}
	return (true);
}

bool	validateRequiredChildren(const Directive* node)
{
	// Find the directive specification
	std::map<std::string, DirectiveDefinition>::const_iterator it = NGINX_DIRECTIVE_SPECS.find(node->getName());
	if (it == NGINX_DIRECTIVE_SPECS.end())
		return (false); // Directive not found in specs

	const DirectiveDefinition& spec = it->second;

	// Check each required child directive
	for (const std::string& requiredChild : spec.requiredChildren)
	{
		bool found = false;
		for (const Directive* child : node->getChildren())
		{
			if (child->getName() == requiredChild)
			{
				found = true;
				break;
			}
		}
		if (!found)
			return (false); // Required child directive missing
	}
	return (true);
}

bool	validateHttpDirective(const Directive* node)
{
	if (node->getChildren().empty())
		return (false);

	// Validate required children are present
	if (!validateRequiredChildren(node))
		return (false);

	// Validate that each child is in the right context
	if (!validateContext(node))
		return (false);

	return (true);
}

bool	validateServerDirective(const Directive* node)
{
	if (node->getChildren().empty())
		return (false);

	// Validate required children are present
	if (!validateRequiredChildren(node))
		return (false);

	if (!validateContext(node))
		return (false);

	return (true);
}

bool	validateLocationDirective(const Directive* node)
{
	if (node->getChildren().empty())
		return (false);

	// Validate required children are present
	if (!validateRequiredChildren(node))
		return (false);

	if (!validateContext(node))
		return (false);

	return (true);
}

bool	validateListenDirective(const Directive* node)
{
	std::pair<std::string, std::string>	addressAndPort;	//127.0.0.0:8080
	bool								isValidAddress;
	bool								isValidPort;

	if (node->getParameters().empty())
		return (false);
	
	addressAndPort = parseAddressAndPort(node->getParameter(0));
	if (addressAndPort.first.empty() && addressAndPort.second.empty())
	{
		if (validateAddress(node->getParameter(0)))
			return (true);
		if (validatePort(node->getParameter(0)))
			return (true);
		return (false);
	}
	isValidAddress = validateAddress(addressAndPort.first);
	isValidPort = validatePort(addressAndPort.second);
	if (isValidAddress && addressAndPort.second.empty())
		return (true);
	else if (isValidAddress && isValidPort)
		return (true);
	else if (addressAndPort.first.empty() && isValidPort)
		return (true);
	return (false);
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
			return (false);

		std::string	quoted_content = path.substr(1, path.length() - 2);

		// Must still start with '/' after removing quotes
		if (quoted_content.empty() || quoted_content[0] != '/')
			return (false);

		return (true);
	}

	// Would need to verify that it's a valid path.

	// Invalid: doesn't start with '/', '"', or '$'
	return (false);
}

bool	validateIndexDirective(const Directive* node)
{
	if (node->getParameters().empty())
		return (false);
	if (node->getParameter(0) == "index.html") //Hardcoded
		return (true);
	return (false);
}

bool	validateAutoIndexDirective(const Directive* node)
{
	if (node->getParameters().empty())
		return (false);
	if (node->getParameter(0) == "on" || node->getParameter(0) == "off")
		return (true);
	return (false);
}

bool	validateErrorPageDirective(const Directive* node)
{
	// error_page 404 /404.html;			The last value is a URI.
	// error_page 500 502 503 /50x.html;	There can be multiple codes.
	// error_page 404 =200 /empty.gif;		An equals changes the code.

	if (node->getParameters().size() < 2)
		return (false);

	// Last parameter is always the URI
	const std::string& uri = node->getParameters().back();

	// Check URI format (should start with / or be a valid URL)
	if (uri.empty() || (uri[0] != '/' && uri.find("http") != 0))
		return (false);

	// All parameters except the last one are error codes
	for (size_t i = 0; i < node->getParameters().size() - 1; ++i)
	{
		const std::string& param = node->getParameter(i);

		// Check for response code change (e.g., "=200")
		if (param[0] == '=')
		{
			if (param.length() < 2)
				return (false);
			try
			{
				int new_code = std::stoi(param.substr(1));
				// Validate it's a valid HTTP status code
				if (new_code < 100 || new_code > 599)
					return (false);
			}
			catch (const std::exception&)
			{
				return (false);
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
					return (false);
			}
			catch (const std::exception&)
			{
				return (false);
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
		return (false);

	// First parameter must be a valid HTTP status code
	try
	{
		int status_code = std::stoi(node->getParameter(0));

		// Must be a valid HTTP status code (100-599)
		if (status_code < 100 || status_code > 599)
			return (false);
	}
	catch (const std::exception&)
	{
		return (false);
	}

	// If there's a second parameter, validate it as URL or text
	if (node->getParameters().size() == 2)
	{
		const std::string& second_param = node->getParameter(1);

		if (second_param.empty())
			return (false);

		// For URLs starting with http:// or https://
		if (second_param.find("http://") == 0 || second_param.find("https://") == 0)
		{
			// Basic URL validation - must have something after protocol
			if (second_param.length() <= 8) // "https://" is 8 chars
				return (false);
		}
		// For quoted text
		else if (second_param.front() == '"' && second_param.back() == '"')
		{
			// Must have content between quotes
			if (second_param.length() < 3)
				return (false);
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
					return (false);
			}

			// Check for consecutive slashes (except at start)
			if (second_param.find("//") != std::string::npos)
				return (false);
		}
		else
		{
			// Other text content is also valid
		}
	}

	return (true);
}

bool	validateLimitExceptDirective(const Directive* node)
{
	// It should take a method as a parameter.
	// If GET is allowed, so is HEAD.
	// It allows these methods inside a location.
	// Within, it should have directives 'allow' and/or 'deny'.

	const std::vector<std::string> httpMethods = {"GET", "POST", "DELETE", "HEAD"};

	if (node->getParameters().empty() || node->getChildren().empty())
		return (false);
	
	for (std::string currentMethod : node->getParameters())
	{
		bool	isValidMethod = false;
		for (std::string allowed : httpMethods)
		{
			if (currentMethod == allowed)
			{
				isValidMethod = true;
				break ;
			}
		}
		if (isValidMethod == false)
			return (false);	// Throw validation error
	}
	//	Validate children
	for (const Directive*	currentChild : node->getChildren())
	{
		if (currentChild->getName() != "allow" && currentChild->getName() != "deny")
			return (false); //Missing directives inside the block
		if (!validateAllowOrDeny(currentChild))
			return (false);
	}
	return (true);
}

bool	validateClientMaxBodySizeDirective(const Directive* node)
{
	// If 0 == no limit
	// Can be 1m or 10m which means 10 megabytes. Any request that exceeds that it returns a 413 error.
	// The letter can be lower or upper case.

	if (node->getParameters().empty())
		return (false);
	if (node->getParameter(0) == "0")
		return (true);
	try
	{
		int	size = std::stoi(node->getParameter(0));
		if (size < 1 || size > 10)
		{
			return (false); //Invalid size
		}
		if (node->getParameter(0).back() != 'm' || node->getParameter(0).back() != 'M')	//Only accepting megabytes as valid sizes
		{
			return (false);	//Unknown size
		}
	}
	catch(const std::exception& e)
	{
		throw std::runtime_error("Failed stoi in validate client max body size");
	}
	
	return (false);
}
