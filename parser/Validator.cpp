#include "../include/Validator.hpp"
#include "../include/DirectiveSpecs.hpp"

Validator::Validator(std::unique_ptr<ConfigFile>& configFile) : _ConfigFile(std::move(configFile)), _directiveSpecs(NGINX_DIRECTIVE_SPECS)
{
	
}
Validator::~Validator()
{

}

bool	Validator::validate()
{
	return (false);
}

bool	Validator::validateUserDirective(const Directive* node)
{
	if (node->name == "user" && !node->parameters.empty())
	{
		// Validate user directive logic here
		return (true);
	}

	return (false);
}


bool	Validator::validateWorkerProcessesDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateHttpDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateServerDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateLocationDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateListenDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateServerNameDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateRootDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateIndexDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateErrorPageDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateFastcgiPassDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateFastcgiParamDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateFastcgiIndexDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateReturnDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateRewriteDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateLimitExceptDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateClientBodyTempPathDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateClientMaxBodySizeDirective(const Directive* node)
{
	return (false);
}
bool	Validator::validateAutoIndexDirective(const Directive* node)
{
	return (false);
}
