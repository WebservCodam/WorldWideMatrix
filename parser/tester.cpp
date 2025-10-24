#include "../include/Lexer.hpp"
#include "../include/Parser.hpp"
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
	std::vector<std::unique_ptr<Directive>>	ast = parser.parse();

	printAST(ast);

	// Validator	validator(ast);

	return (0);
}
