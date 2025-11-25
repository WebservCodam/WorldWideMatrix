#include "../include/Configuration.hpp"
#include "../include/Lexer.hpp"
#include "../include/Parser.hpp"
#include "../include/ConfigError.hpp"
#include "PrintUtilities.hpp"
#include "../include/Validator.hpp"

int	main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Error: Expecting an input file." << std::endl;
		exit(EXIT_FAILURE);
	}

	std::ifstream file(argv[1]);
	if (!file)
	{
		std::cerr << "Error: Could not open file" << std::endl;
		exit(EXIT_FAILURE);
	}

	std::stringstream	buffer;
	buffer << file.rdbuf();
	std::string input = buffer.str();

	try
	{
		// Phase 1: Lexing
		std::vector<Token> tokenList = Lexer::tokenize(input);
		// printTokensList(tokenList);

		// Phase 2: Parsing
		Parser parser = Parser(tokenList);
		std::unique_ptr<ConfigFile>	ast = parser.parse();

		if (!ast)
		{
			std::cerr << "Error: Failed to parse configuration" << std::endl;
			return (EXIT_FAILURE);
		}

		// printAST(ast);

		// Phase 3: Validation
		Validator	validator(ast);
		if (validator.validate())
		{
			std::cout << "Configuration is valid!" << std::endl;
			return (EXIT_SUCCESS);
		}
		else
		{
			std::cerr << "Error: Configuration validation failed" << std::endl;
			return (EXIT_FAILURE);
		}
	}
	catch (const ConfigError& e)
	{
		std::cerr << e.what() << std::endl;
		return (EXIT_FAILURE);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Unexpected error: " << e.what() << std::endl;
		return (EXIT_FAILURE);
	}
}
