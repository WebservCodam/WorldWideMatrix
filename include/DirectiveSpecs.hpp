#pragma once

#include "Configuration.hpp"

// bool	validateUserDirective(const Directive* node);
bool	validateWorkerProcessesDirective(const Directive* node);
bool	validateHttpDirective(const Directive* node);
bool	validateServerDirective(const Directive* node);
bool	validateLocationDirective(const Directive* node);
bool	validateListenDirective(const Directive* node);
bool	validateServerNameDirective(const Directive* node);
bool	validateRootDirective(const Directive* node);
bool	validateIndexDirective(const Directive* node);
bool	validateErrorPageDirective(const Directive* node);
bool	validateFastcgiPassDirective(const Directive* node);
bool	validateFastcgiParamDirective(const Directive* node);
bool	validateFastcgiIndexDirective(const Directive* node);
bool	validateReturnDirective(const Directive* node);
bool	validateRewriteDirective(const Directive* node);
bool	validateLimitExceptDirective(const Directive* node);
bool	validateClientBodyTempPathDirective(const Directive* node);
bool	validateClientMaxBodySizeDirective(const Directive* node);
bool	validateAutoIndexDirective(const Directive* node);

extern const std::map<std::string, DirectiveDefinition> NGINX_DIRECTIVE_SPECS;

