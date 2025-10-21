#pragma once

#include "Configuration.hpp"

class	Validator
{
	private:
		std::unique_ptr<ConfigFile>	_ConfigFile;
		
		struct DirectiveDefinition
		{
			std::string	name;
			bool		isBlock;
			size_t		minArgs;
			size_t		maxArgs;
			std::set<std::string>	validContexts;
			std::function<bool(const std::vector<std::string>&)> validateArgs;
		};

		std::map<std::string, DirectiveDefinition> _directiveSpecs;

	public:
		Validator() = delete;
		Validator(std::unique_ptr<ConfigFile>& configFile);
		~Validator();

		bool	validate();

	private:

};