#include "../../include/Configuration.hpp"

bool	validateReturnDirective(Directive* node)
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
