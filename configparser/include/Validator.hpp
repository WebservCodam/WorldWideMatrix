#pragma once

#include "Configuration.hpp"
#include "DirectiveSpecs.hpp"

class	Validator
{
	private:
		const ConfigFile*							_ConfigFile;
		std::map<std::string, DirectiveDefinition>	_directiveSpecs;

	public:
		Validator() = delete;
		Validator(const ConfigFile* configFile);
		~Validator() = default;

		bool	validate();

	private:
		bool	validateDirective(const Directive* node);
};
