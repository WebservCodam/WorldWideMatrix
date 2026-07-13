#include "../Configuration.hpp"
#include <cctype>
#include <algorithm>

void	validateClientMaxBodySizeDirective(Directive* node)
{
	const std::string&	param = node->getParameter(0);

	if (param.empty())
		throw ConfigError::validation("Invalid size value in '" + node->getName() + "' directive", node);

	// Split the leading digits from the unit suffix.
	size_t	i = 0;
	while (i < param.size() && std::isdigit(static_cast<unsigned char>(param[i])))
		i++;
	std::string	digits = param.substr(0, i);
	std::string	unit = param.substr(i);
	std::transform(unit.begin(), unit.end(), unit.begin(), ::tolower);

	if (digits.empty())
		throw ConfigError::validation("Invalid size format in '" + node->getName() + "' directive: '" + param + "'", node);

	unsigned long long	multiplier;
	if (unit.empty() || unit == "b")
		multiplier = 1;
	else if (unit == "k" || unit == "kb")
		multiplier = 1000;
	else if (unit == "m" || unit == "mb")
		multiplier = 1000000;
	else
		throw ConfigError::validation("Invalid size unit in '" + node->getName() + "' directive: '" + param + "'", node);

	unsigned long long	value;
	try
	{
		value = std::stoull(digits) * multiplier;
	}
	catch (const std::exception&)
	{
		throw ConfigError::validation("Invalid size format in '" + node->getName() + "' directive: '" + param + "'", node);
	}

	node->setParameter(0, std::to_string(value));
}

unsigned long long	ConfigFile::processClientMaxBodySize(const Directive* directive)
{
	if (directive)
	{
		return (std::stoull(directive->getParameter(0)));
	}
	return (DEFAULT_MAX_BODY_SIZE);
}
