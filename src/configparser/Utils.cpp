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
