#include "../../include/Configuration.hpp"

bool	validateErrorPageDirective(Directive* node)
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
