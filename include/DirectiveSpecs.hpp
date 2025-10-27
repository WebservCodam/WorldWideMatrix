#pragma once

#include "Configuration.hpp"

// Forward declarations of validation functions
bool validateWorkerProcessesArgs(const Directive* node);

extern const std::map<std::string, DirectiveDefinition> NGINX_DIRECTIVE_SPECS;

