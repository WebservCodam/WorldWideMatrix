#include "Validator.hpp"

Validator::Validator(std::unique_ptr<ConfigFile>& configFile)
{
	this->_ConfigFile = std::move(configFile);
}
Validator::~Validator()
{

}

void	Validator::validateSyntax()
{
	// Validate name
	// Validate argument count
	// Are semicolons, brackets and curly braces well placed? 
}
void	Validator::validateSemantics()
{
	// Are the argument types correct?
	// Are the values within the correct ranges?
	// Are the directives in the right context?
}
void	Validator::validateContext()
{
	// Are there conflicting directives?
	// Are there dependencies within directives?
}

bool	validateName()
{
	
}
