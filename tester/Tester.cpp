#include "../include/Configuration.hpp"
#include "../include/Lexer.hpp"
#include "../include/Parser.hpp"
#include "../include/ParseError.hpp"
#include "../include/PrintUtilities.hpp"
#include "../include/Utilities.hpp"
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

	std::vector<Token> tokenList = Lexer::tokenize(input);

    // printTokensList(tokenList);

	Parser parser = Parser(tokenList);
	std::unique_ptr<ConfigFile>	ast = parser.parse();

	printAST(ast);

	Validator	validator(ast);

	return (0);

	// // Main usage:
	// try
	// {
	// 	// Phase 1: Lexing
	// 	Lexer lexer(input, filename);
	// 	std::vector<Token> tokens = lexer.tokenize();

	// 	// Phase 2: Parsing
	// 	Parser parser(tokens);
	// 	std::unique_ptr<ConfigFile> config = parser.parse();

	// 	// Phase 3: Validation
	// 	Validator validator(config.get());  // Pass pointer, not ownership
	// 	validator.validate();  // Throws on error

	// 	// Phase 4: Use validated config
	// 	// ...

	// } catch (const ParseError& e) {
	// 	std::cerr << e.what() << std::endl;
	// } catch (const ValidationError& e) {
	// 	std::cerr << e.what() << std::endl;
	// }
}

