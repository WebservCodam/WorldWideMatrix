#include <Parser.hpp>
#include <Lexer.hpp>
#include <iostream>

void findDirectives(const ASTNode* node, const std::string& name)
{
	const SimpleDirective* simpleDir = dynamic_cast<const SimpleDirective*>(node);
	const BlockDirective* blockDir = dynamic_cast<const BlockDirective*>(node);
	const ConfigFile* config = dynamic_cast<const ConfigFile*>(node);

	if (simpleDir && simpleDir->name == name)
	{
		std::cout << "Found " << name << ":";
		for (const auto& param : simpleDir->parameters)
			std::cout << " " << param;
		std::cout << std::endl;
	}
	else if (blockDir && blockDir->name == name)
	{
		std::cout << "Found " << name << " block";
		if (!blockDir->parameters.empty())
		{
			std::cout << " with parameters:";
			for (const auto& param : blockDir->parameters)
				std::cout << " " << param;
		}
		std::cout << std::endl;
	}

	if (blockDir)
	{
		for (const auto& child : blockDir->children)
			findDirectives(child.get(), name);
	}
	else if (config)
	{
		for (const auto& directive : config->directives)
			findDirectives(directive.get(), name);
	}
}

int main()
{
	std::string configText = R"(
		user nginx;
		worker_processes auto;

		events {
			worker_connections 1024;
		}

		http {
			server {
				listen 80;
				server_name example.com;

				location / {
					root /var/www;
					index index.html;
				}
			}
		}
	)";

	try
	{
		Lexer lexer;
		auto tokens = lexer.tokenize(configText);

		Parser parser;
		auto ast = parser.parse(tokens);

		std::cout << "=== Searching for 'listen' directives ===" << std::endl;
		findDirectives(ast.get(), "listen");

		std::cout << "\n=== Searching for 'server' blocks ===" << std::endl;
		findDirectives(ast.get(), "server");

	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}