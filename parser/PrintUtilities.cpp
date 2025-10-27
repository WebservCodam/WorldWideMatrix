#include "../include/Configuration.hpp"

inline const char*		tokenTypeToString(TokenType type);
inline std::ostream&	operator<<(std::ostream& os, const Token& token);
void					printTokensList(const std::vector<Token>& tokenList);
void					printIndent(int indent, const std::string& prefix);
void					printDirective(const Directive* directive, int indent, const std::string& prefix);
void					printAST(const std::vector<std::unique_ptr<Directive>>& config);

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
        case STRING:      return "STRING";
        case LBRACKET:    return "LBRACKET";
        case RBRACKET:    return "RBRACKET";
        case COMMA:       return "COMMA";
        // case COLON:       return "COLON";
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

void    printIndent(int indent, const std::string& prefix)
{
    for (int i = 0; i < indent; i++)
    {
        std::cout << "  ";
    }
    std::cout << prefix;
}

void    printDirective(const Directive* directive, int indent, const std::string& prefix)
{
    if (!directive)
    {
        printIndent(indent, prefix);
        std::cout << "(null)" << std::endl;
        return;
    }

    printIndent(indent, prefix);

    // Determine if it's a simple or block directive based on children
    if (directive->children.empty())
    {
        std::cout << "SimpleDirective: " << directive->name;
    }
    else
    {
        std::cout << "BlockDirective: " << directive->name;
    }

    std::cout << " [line: " << directive->line
              << ", col: " << directive->column
              << ", context: " << directive->context << "]" << std::endl;

    // Print parameters if any
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

    // Print children if any (for block directives)
    if (!directive->children.empty())
    {
        printIndent(indent + 1, "");
        std::cout << "Children:" << std::endl;
        for (size_t i = 0; i < directive->children.size(); i++)
        {
            bool isLast = (i == directive->children.size() - 1);
            std::string childPrefix = isLast ? "└── " : "├── ";
            printDirective(directive->children[i].get(), indent + 2, childPrefix);
        }
    }
}

void    printAST(const std::vector<std::unique_ptr<Directive>>& config)
{
    std::cout << "\n=== AST Structure ===" << std::endl;
    if (!config.empty())
    {
        for (size_t i = 0; i < config.size(); i++)
        {
            bool isLast = (i == config.size() - 1);
            std::string prefix = isLast ? "└── " : "├── ";
            printDirective(config[i].get(), 0, prefix);
        }
    }
    else
    {
        std::cout << "(empty)" << std::endl;
    }
    std::cout << "====================" << std::endl;
}
