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

bool	validateUserDirective();
bool	validateWorkerProcessesDirective();
bool	validateHttpDirective();
bool	validateServerDirective();
bool	validateLocationDirective();
bool	validateListenDirective();
bool	validateServerNameDirective();
bool	validateRootDirective();
bool	validateIndexDirective();
bool	validateErrorPageDirective();
bool	validateFastcgiPassDirective();
bool	validateFastcgiParamDirective();
bool	validateFastcgiIndexDirective();
bool	validateReturnDirective();
bool	validateRewriteDirective();
bool	validateLimitExceptDirective();
bool	validateClientBodyTempPathDirective();
bool	validateClientMaxBodySizeDirective();
bool	validateAutoIndexDirective();
