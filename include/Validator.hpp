#pragma once

#include "Configuration.hpp"
#include "DirectiveSpecs.hpp"

class	Validator
{
	private:
		std::unique_ptr<ConfigFile>					_ConfigFile;
		std::map<std::string, DirectiveDefinition>	_directiveSpecs;

	public:
		Validator() = delete;
		Validator(std::unique_ptr<ConfigFile>& configFile);
		~Validator();

		bool	validate();

	private:

};