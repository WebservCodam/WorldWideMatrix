#include <Parser.hpp>
#include <Lexer.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

void printAST(const ASTNode* node, int indent = 0);
void printIndent(int indent);

void printIndent(int indent)
{
	for (int i = 0; i < indent; i++)
		std::cout << "  ";
}

void printAST(const ASTNode* node, int indent)
{
	if (!node)
		return;

	const SimpleDirective* simpleDir = dynamic_cast<const SimpleDirective*>(node);
	const BlockDirective* blockDir = dynamic_cast<const BlockDirective*>(node);
	const ConfigFile* config = dynamic_cast<const ConfigFile*>(node);

	if (simpleDir)
	{
		printIndent(indent);
		std::cout << simpleDir->name;
		for (const auto& param : simpleDir->parameters)
			std::cout << " " << param;
		std::cout << ";" << std::endl;
	}
	else if (blockDir)
	{
		printIndent(indent);
		std::cout << blockDir->name;
		for (const auto& param : blockDir->parameters)
			std::cout << " " << param;
		std::cout << " {" << std::endl;

		for (const auto& child : blockDir->children)
			printAST(child.get(), indent + 1);

		printIndent(indent);
		std::cout << "}" << std::endl;
	}
	else if (config)
	{
		std::cout << "Configuration File:" << std::endl;
		for (const auto& directive : config->directives)
			printAST(directive.get(), 0);
	}
}

std::string readFile(const std::string& filename)
{
	std::ifstream file(filename);
	if (!file.is_open())
		throw std::runtime_error("Cannot open file: " + filename);

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
		return 1;
	}

	try
	{
		std::string content = readFile(argv[1]);
		std::cout << "Parsing file: " << argv[1] << std::endl;
		std::cout << "----------------------------------------" << std::endl;

		Lexer lexer;
		auto tokens = lexer.tokenize(content);

		std::cout << "Tokens generated: " << tokens.size() << std::endl;
		std::cout << "----------------------------------------" << std::endl;

		Parser parser;
		auto ast = parser.parse(tokens);

		std::cout << "Parsing successful!" << std::endl;
		std::cout << "----------------------------------------" << std::endl;
		printAST(ast.get());

	}
	catch (const ParseError& e)
	{
		std::cerr << "Parse Error: " << e.what() << std::endl;
		return 1;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}