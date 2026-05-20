#include "../Configuration.hpp"

void	validateMethodsDirective(Directive* node)
{
	const std::vector<std::string>	httpMethods = {"GET", "POST", "DELETE"};
	const std::vector<std::string>	methods = node->getParameters();

	for (const std::string& method : methods)
	{
		bool	valid = false;
		for (const std::string& httpMethod : httpMethods)
		{
			if (method == httpMethod)
				valid = true;
		}
		if (valid == false)
			throw ConfigError::validation("The provided method: '" + method + "' is invalid.", node);
	}
}
