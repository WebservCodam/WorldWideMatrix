#include "Configuration.hpp"

std::string	joinPath(const std::string& a, const std::string& b)
{
	if (a.empty())
		return (b);
	if (b.empty())
		return (a);
	if (a.back() == '/' && b.front() == '/')
		return (a + b.substr(1));
	if (a.back() != '/' && b.front() != '/')
		return (a + "/" + b);
	return (a + b);
}

void	checkPath(const std::string& path, ErrorType errorType, const std::string& msg1, const std::string& msg2)
{
	struct stat	st;

	// Check URIs
	if (stat(std::string(path).c_str(), &st) != 0)
		throw ConfigError(errorType, msg1);
	if (access(path.c_str(), R_OK) != 0)
		throw ConfigError(errorType, msg2);
}
