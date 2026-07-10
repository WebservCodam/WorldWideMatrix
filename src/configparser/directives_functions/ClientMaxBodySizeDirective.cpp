#include "../Configuration.hpp"
#include <cctype>
#include <algorithm>

void	validateClientMaxBodySizeDirective(Directive* node)
{
	// Accepts a byte count with an optional, case-insensitive unit suffix:
	//   (none)/b = bytes, k/kb = *1000, m/mb = *1000000.  "0" means no limit.
	// The value is normalized to a plain byte count and written back, so the
	// rest of the config code can just stoull() it. No upper limit is imposed;
	// a per-location override may legitimately be larger than the default.
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

/**
 * @brief
 * During validation the valid parameters that had letters have been changed to the number equivalent.
 * @return
 * This function simply returns the conversion of that number stored in a string into an unsigned long long.
 */
unsigned long long	ConfigFile::processClientMaxBodySize(const Directive* directive)
{
	if (directive)
	{
		return (std::stoull(directive->getParameter(0)));
	}
	return (DEFAULT_MAX_BODY_SIZE);
}
