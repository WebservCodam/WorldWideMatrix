#include "../Configuration.hpp"

void	validateCgiPassDirective(Directive* node)
{
	// Sets the address for a CGI server.
	// The address can be specified as a domain name or IP address, and a port (e.g.): localhost:9000;
	// If a domain name resolves to several addresses, all of them will be used in a round-robin fashion. But we don't need that...
	return ;
}

void	validateCgiParamDirective(Directive* node)
{
	// Sets a parameter that should be passed to the CGI server.
	// The value can contain text, variables, and their combination.

	// If the directive is specified with if_not_empty
	// then such a parameter will be passed to the server only if its value is not empty.
	// We can skip that.

	return ;
}

void	validateCgiIndexDirective(Directive* node)
{
	// Sets a file name that will be appended after a URI that ends with a slash,
	// in the value of the $cgi_script_name variable.
	// In other words, it sets a value for the variable $cgi_script_name.
	return ;
}
