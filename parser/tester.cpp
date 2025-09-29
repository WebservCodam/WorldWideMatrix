#include <../include/Lexer.hpp>
#include <../include/Parser.hpp>

inline const char* tokenTypeToString(TokenType type)
{
	switch (type)
	{
		case WORD:        return "WORD";
		case NUMBER:      return "NUMBER";
		case LBRACE:      return "LBRACE";
		case RBRACE:      return "RBRACE";
		case SEMICOLON:   return "SEMICOLON";
		case COMMENT:     return "COMMENT";
		case END_OF_FILE: return "END_OF_FILE";
		default:          return "UNKNOWN";
	}
}

inline std::ostream& operator<<(std::ostream& os, const Token& token)
{
	os << "Token(" 
	   << tokenTypeToString(token.type)
	   << ", value=\"" << token.value << "\""
	   << ", line=" << token.line
	   << ", column=" << token.column
	   << ")";
	return os;
}

void	printTokensList(const std::vector<Token>& tokenList)
{
	 for (const Token& token : tokenList)
    {
        std::cout << token << std::endl;
    }
}

void printASTNode(const ASTNode* node, int indent = 0, const std::string& prefix = "");

void printIndent(int indent, const std::string& prefix = "")
{
    for (int i = 0; i < indent; i++)
    {
        std::cout << "  ";
    }
    std::cout << prefix;
}

void printSimpleDirective(const SimpleDirective* directive, int indent, const std::string& prefix)
{
    printIndent(indent, prefix);
    std::cout << "SimpleDirective: " << directive->name 
              << " [line:" << directive->line 
              << ", col:" << directive->column << "]" << std::endl;
    
    if (!directive->parameters.empty())
    {
        printIndent(indent + 1, "");
        std::cout << "Parameters: ";
        for (size_t i = 0; i < directive->parameters.size(); i++)
        {
            std::cout << "\"" << directive->parameters[i] << "\"";
            if (i < directive->parameters.size() - 1)
                std::cout << ", ";
        }
        std::cout << std::endl;
    }
}

void printBlockDirective(const BlockDirective* directive, int indent, const std::string& prefix)
{
    printIndent(indent, prefix);
    std::cout << "BlockDirective: " << directive->name 
              << " [line:" << directive->line 
              << ", col:" << directive->column << "]" << std::endl;
    
    if (!directive->parameters.empty())
    {
        printIndent(indent + 1, "");
        std::cout << "Parameters: ";
        for (size_t i = 0; i < directive->parameters.size(); i++)
        {
            std::cout << "\"" << directive->parameters[i] << "\"";
            if (i < directive->parameters.size() - 1)
                std::cout << ", ";
        }
        std::cout << std::endl;
    }
    
    if (!directive->children.empty())
    {
        printIndent(indent + 1, "");
        std::cout << "Children:" << std::endl;
        for (size_t i = 0; i < directive->children.size(); i++)
        {
            bool isLast = (i == directive->children.size() - 1);
            std::string childPrefix = isLast ? "└── " : "├── ";
            printASTNode(directive->children[i].get(), indent + 2, childPrefix);
        }
    }
}

void printASTNode(const ASTNode* node, int indent, const std::string& prefix)
{
    if (!node)
    {
        printIndent(indent, prefix);
        std::cout << "(null)" << std::endl;
        return;
    }
    
    // Try to downcast to specific types
    if (const SimpleDirective* simple = dynamic_cast<const SimpleDirective*>(node))
    {
        printSimpleDirective(simple, indent, prefix);
    }
    else if (const BlockDirective* block = dynamic_cast<const BlockDirective*>(node))
    {
        printBlockDirective(block, indent, prefix);
    }
    else if (const ConfigFile* config = dynamic_cast<const ConfigFile*>(node))
    {
        printIndent(indent, prefix);
        std::cout << "ConfigFile [line:" << config->line 
                  << ", col:" << config->column << "]" << std::endl;
        
        for (size_t i = 0; i < config->directives.size(); i++)
        {
            bool isLast = (i == config->directives.size() - 1);
            std::string childPrefix = isLast ? "└── " : "├── ";
            printASTNode(config->directives[i].get(), indent + 1, childPrefix);
        }
    }
    else
    {
        printIndent(indent, prefix);
        std::cout << "Unknown ASTNode [line:" << node->line 
                  << ", col:" << node->column << "]" << std::endl;
    }
}

void printAST(const std::unique_ptr<ConfigFile>& config)
{
    std::cout << "\n=== AST Structure ===" << std::endl;
    if (config)
    {
        printASTNode(config.get(), 0, "");
    }
    else
    {
        std::cout << "(empty)" << std::endl;
    }
    std::cout << "====================" << std::endl;
}

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

	Parser parser = Parser(tokenList);
	std::unique_ptr<ConfigFile> ast = parser.parse();

	printAST(ast);

	return (0);
}
