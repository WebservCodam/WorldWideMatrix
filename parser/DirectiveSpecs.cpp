#include "../include/DirectiveSpecs.hpp"

const std::map<std::string, DirectiveDefinition> NGINX_DIRECTIVE_SPECS = // The equals explicitly says that this is an initialization.
{
	//	=== Main Context Directives ===
	{"user", {"user", false, 0, 2, {"main"}, nullptr}},
	{"worker_processes", {"worker_processes", false, 1, 1, {"main"}, validateWorkerProcessesDirective}},

	//	===	Block Directives ===
	{"http", {"http", true, 0, 0, {"main"}, nullptr}},
	{"server", {"server", true, 0, 0, {"http"}, nullptr}},
	{"location", {"location", true, 1, 2, {"server", "location"}, nullptr}},	// 2 parameters in case it's an equals

	//	=== Server Basics ===
	{"listen", {"listen", false, 1, 1, {"server"}, nullptr}}, //Only taking addresses and ports. It can be either/and. If it's both then it's separated by ':'.
	{"server_name", {"server_name", false, 1, 100, {"server"}, nullptr}},
	{"root", {"root", false, 1, 1, {"http", "server", "location"}, nullptr}},
	{"index", {"index", false, 1, 100, {"http", "server", "location"}, nullptr}},

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
	{"client_max_body_size", {"client_max_body_size", false, 1, 1, {"http", "server", "location"}, nullptr}},

	//	=== Autoindex ===
	{"autoindex", {"autoindex", false, 1, 1, {"http", "server", "location"}, nullptr}},  // on/off
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
	try
	{
		int port = std::stoi(node->parameters.at(0));
		
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
	return (false);
}
bool	validateServerNameDirective(const Directive* node)
{
	return (false);
}
bool	validateRootDirective(const Directive* node)
{
	return (false);
}
bool	validateIndexDirective(const Directive* node)
{
	return (false);
}
bool	validateErrorPageDirective(const Directive* node)
{
	return (false);
}
bool	validateFastcgiPassDirective(const Directive* node)
{
	return (false);
}
bool	validateFastcgiParamDirective(const Directive* node)
{
	return (false);
}
bool	validateFastcgiIndexDirective(const Directive* node)
{
	return (false);
}
bool	validateReturnDirective(const Directive* node)
{
	return (false);
}
bool	validateRewriteDirective(const Directive* node)
{
	return (false);
}
bool	validateLimitExceptDirective(const Directive* node)
{
	return (false);
}
bool	validateClientBodyTempPathDirective(const Directive* node)
{
	return (false);
}
bool	validateClientMaxBodySizeDirective(const Directive* node)
{
	return (false);
}
bool	validateAutoIndexDirective(const Directive* node)
{
	return (false);
}