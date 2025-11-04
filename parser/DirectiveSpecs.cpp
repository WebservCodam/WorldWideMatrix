#include "../include/DirectiveSpecs.hpp"

const std::map<std::string, DirectiveDefinition> NGINX_DIRECTIVE_SPECS = // The equals explicitly says that this is an initialization.
{
	//	=== Main Context Directives ===
	{"user", {"user", false, 0, 2, {"main"}, nullptr}},
	{"worker_processes", {"worker_processes", false, 1, 1, {"main"}, validateWorkerProcessesDirective}},

	//	===	Block Directives ===
	{"http", {"http", true, 0, 0, {"main"}, validateHttpDirective}},
	{"server", {"server", true, 0, 0, {"http"}, validateServerDirective}},
	{"location", {"location", true, 1, 2, {"server", "location"}, validateLocationDirective}},	// 2 parameters in case it's an equals

	//	=== Server Basics ===
	{"listen", {"listen", false, 1, 1, {"server"}, validateListenDirective}}, //Only taking addresses and ports. It can be either/and. If it's both then it's separated by ':'.
	{"server_name", {"server_name", false, 1, 100, {"server"}, nullptr}},
	{"root", {"root", false, 1, 1, {"http", "server", "location"}, validateRootDirective}},
	// {"index", {"index", false, 1, 100, {"http", "server", "location"}, nullptr}},

	//	=== Autoindex ===
	{"autoindex", {"autoindex", false, 1, 1, {"http", "server", "location"}, validateAutoIndexDirective}},  // on/off

	//	=== Error Handling ===
	{"error_page", {"error_page", false, 2, 100, {"http", "server", "location"}, nullptr}},

	// === CGI ===
	{"fastcgi_pass", {"fastcgi_pass", false, 1, 1, {"location"}, nullptr}},
	{"fastcgi_param", {"fastcgi_param", false, 2, 3, {"http", "server", "location"}, nullptr}},
	{"fastcgi_index", {"fastcgi_index", false, 1, 1, {"http", "server", "location"}, nullptr}},

	//	===	Request Handling ===
	{"return", {"return", false, 1, 2, {"server", "location"}, nullptr}},	// 301, 302 redirects
    {"rewrite", {"rewrite", false, 2, 4, {"server", "location"}, nullptr}},

	//	=== Methods/Limits	===
	{"limit_except", {"limit_except", true, 1, 10, {"location"}, nullptr}},	// GET, POST, DELETE
    {"client_body_temp_path", {"client_body_temp_path", false, 1, 1, {"http", "server", "location"}, nullptr}},
	{"client_max_body_size", {"client_max_body_size", false, 1, 1, {"http", "server", "location"}, nullptr}}
};

// bool	Validator::validateUserDirective(const Directive* node)
// {
// 	return validateDirective(node);
// }

// Validation functions for specific directives
bool	validateWorkerProcessesDirective(const Directive* node)
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
bool	validateHttpDirective(const Directive* node)
{
	if (node->children.empty())
		return (false);
	else
	{
		// validate children directives
		return (true);
	}
}
bool	validateServerDirective(const Directive* node)
{
	if (node->children.empty())
		return (false);
	else
	{
		// validate children directives
		return (true);
	}
}
bool	validateLocationDirective(const Directive* node)
{
	if (node->children.empty())
		return (false);
	else
	{
		// validate children directives
		return (true);
	}
}
bool	validateListenDirective(const Directive* node)
{
	//Divide IP from Port
		// Check that the IP address is valid.
		// Localhost can also be a valid address
	
	//Check IP if present

	//Check Port if present: from 1-65535


	try //Only port
	{
		int port = std::stoi(node->parameters.at(0));
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
	return (false);
}
// bool	validateServerNameDirective(const Directive* node)
// {
// 	return (false);
// }


//	Default root would be the root at the http block. (To do after validation)
bool	validateRootDirective(const Directive* node)
{
	// If quoted it's because it may contain a space. But same rules apply:
	// If a variable is inside quotes it doesn't expand, otherwise it does.
	// If it is only variable, it has to resolve to an actual path.
	// A path must begin with a '/'

	const std::string&	path = node->parameters.at(0);

	// Case 1: Path starts with '/' (regular path that may contain variables)
	if (path[0] == '/')
	{
		// TODO: Check if path contains variables ($variable_name)
		// If it does, resolve those variables and validate the resulting path
		// For now, just validate that it starts with '/'
		return (true);
	}

	// Case 2: Path starts with '"' (quoted path, may contain spaces, no variable expansion)
	else if (path[0] == '"')
	{
		// Extract the content between quotes
		if (path.length() < 3 || path.back() != '"')
			return (false);

		std::string	quoted_content = path.substr(1, path.length() - 2);

		// Must still start with '/' after removing quotes
		if (quoted_content.empty() || quoted_content[0] != '/')
			return (false);

		// Quoted paths should not contain variables (they don't expand)
		return (true);
	}

	// Case 3: Path starts with '$' (variable that must resolve to a path starting with '/')
	else if (path[0] == '$')
	{
		// TODO: Resolve the variable and check that it starts with '/'
		// For now, just validate the variable format
		if (path.length() < 2)
			return (false);

		// Variable name should contain only alphanumeric characters and underscores
		for (size_t i = 1; i < path.length(); ++i)
		{
			if (!std::isalnum(path[i]) && path[i] != '_')
				return (false);
		}

		// TODO: Actually resolve the variable and validate the result starts with '/'
		return (true);
	}

	// Invalid: doesn't start with '/', '"', or '$'
	return (false);
}

// We could simply have autoindex on
// bool	validateIndexDirective(const Directive* node)
// {
// 	//List that should be used as indices. Files are checked in the specific order.

// 	return (false);
// }

bool	validateAutoIndexDirective(const Directive* node)
{
	if (node->parameters.at(0) == "on" || node->parameters.at(0) == "off")
		return (true);
	return (false);
}

bool	validateErrorPageDirective(const Directive* node)
{
	// If a value has an equals then it means that the response code is changed.
	// It can have a bunch of values, all of which are error codes. So these have to be valid.
	// The last value is an URI, and it's what the server serves instead of the failing request.
	return (false);
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
	// It returns the code that's specified as value.
	// Optionally there's a 2nd argument that's a text or URL.
	// NGINX also allows to only return a URL, but we can skip that.
	// A URL may contain variables.

	return (false);
}

// bool	validateRewriteDirective(const Directive* node)
// {
//	// Because it uses regex, according to the subject it's not needed to be handled.
// 	return (false);
// }

bool	validateLimitExceptDirective(const Directive* node)
{
	// It should take a method as a parameter. 
	// If GET is allowed, so is HEAD.
	// It allows these methods inside a location.
	// Within, it should have directives 'allow' and/or 'deny'.
	return (false);
}

bool	validateClientBodyTempPathDirective(const Directive* node)
{
	// If it can create a directory with path then it should be valid, otherwise no.
	return (false);
}

bool	validateClientMaxBodySizeDirective(const Directive* node)
{
	// If 0 == no limit
	// Can be 1m or 10m which means 10 megabytes. Any request that exceeds that it returns a 413 error.
	// The letter can be lower or upper case.
	return (false);
}
