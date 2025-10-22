#include "../include/DirectiveSpecs.hpp"

const std::map<std::string, DirectiveDefinition> NGINX_DIRECTIVE_SPECS = // The equals explicitly says that this is an initialization.
{
	//	=== Main Context Directives ===
	{"user", {"user", false, 1, 2, {"main"}}},
	{"worker_processes", {"worker_processes", false, 1, 1, {"main"}}},

	//	===	Block Directives === 
	{"http", {"http", true, 0, 0, {"main"}}},
	{"server", {"server", true, 0, 0, {"http"}}},
	{"location", {"location", true, 1, 2, {"server", "location"}}},	// 2 parameters in case it's an equals

	//	=== Server Basics ===
	{"listen", {"listen", false, 1, 10, {"server"}}},
	{"server_name", {"server_name", false, 1, 100, {"server"}}},
	{"root", {"root", false, 1, 1, {"http", "server", "location"}}},
	{"index", {"index", false, 1, 100, {"http", "server", "location"}}},

	//	=== Error Handling ===
	{"error_page", {"error_page", false, 2, 100, {"http", "server", "location"}}},

	// === CGI ===
	{"fastcgi_pass", {"fastcgi_pass", false, 1, 1, {"location"}}},
	{"fastcgi_param", {"fastcgi_param", false, 2, 3, {"http", "server", "location"}}},
	{"fastcgi_index", {"fastcgi_index", false, 1, 1, {"http", "server", "location"}}},

	//	===	Request Handling ===
	{"return", {"return", false, 1, 2, {"server", "location"}}},	// 301, 302 redirects
    {"rewrite", {"rewrite", false, 2, 4, {"server", "location"}}},

	//	=== Methods/Limits	===
	{"limit_except", {"limit_except", true, 1, 10, {"location"}}},	// GET, POST, DELETE
    {"client_body_temp_path", {"client_body_temp_path", false, 1, 1, {"http", "server", "location"}}},
	{"client_max_body_size", {"client_max_body_size", false, 1, 1, {"http", "server", "location"}}},

	//	=== Autoindex ===
	{"autoindex", {"autoindex", false, 1, 1, {"http", "server", "location"}}},  // on/off
};