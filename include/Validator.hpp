#pragma once

#include "Configuration.hpp"

class	Validator
{
	private:
		std::unique_ptr<ConfigFile>	_ConfigFile;
		bool						_isValidated = true;

	public:
		Validator() = delete;
		Validator(std::unique_ptr<ConfigFile>& configFile);
		~Validator();

		void	validateSyntax();
		void	validateSemantics();
		void	validateContext();
};