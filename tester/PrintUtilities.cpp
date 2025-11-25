#include "../include/Configuration.hpp"

inline const char*		tokenTypeToString(TokenType type);
inline std::ostream&	operator<<(std::ostream& os, const Token& token);
void					printTokensList(const std::vector<Token>& tokenList);
void					printIndent(int indent, const std::string& prefix);
void					printDirective(const Directive* directive, int indent, const std::string& prefix);
void					printAST(const std::unique_ptr<ConfigFile>& config);

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
	return (os);
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
    if (directive->getChildren().empty())
    {
        std::cout << "SimpleDirective: " << directive->getName();
    }
    else
    {
        std::cout << "BlockDirective: " << directive->getName();
    }

    std::cout << " [line: " << directive->getLine()
              << ", col: " << directive->getColumn()
              << ", context: " << directive->getContext() << "]" << std::endl;

    // Print parameters if any
    if (!directive->getParameters().empty())
    {
        printIndent(indent + 1, "");
        std::cout << "Parameters: ";
        for (size_t i = 0; i < directive->getParameters().size(); i++)
        {
            std::cout << "\"" << directive->getParameters()[i] << "\"";
            if (i < directive->getParameters().size() - 1)
                std::cout << ", ";
        }
        std::cout << std::endl;
    }

    // Print children if any (for block directives)
    if (!directive->getChildren().empty())
    {
        printIndent(indent + 1, "");
        std::cout << "Children:" << std::endl;
        for (size_t i = 0; i < directive->getChildren().size(); i++)
        {
            bool isLast = (i == directive->getChildren().size() - 1);
            std::string childPrefix = isLast ? "└── " : "├── ";
            printDirective(directive->getChild(i), indent + 2, childPrefix);
        }
    }
}

void    printAST(const std::unique_ptr<ConfigFile>& config)
{
    std::cout << "\n=== AST Structure ===" << std::endl;

    const std::vector<std::unique_ptr<Directive>>& directives = config.get()->getDirectives();

    if (!directives.empty())
    {
        for (size_t i = 0; i < directives.size(); i++)
        {
            bool isLast = (i == directives.size() - 1);
            std::string prefix = isLast ? "└── " : "├── ";
            printDirective(directives[i].get(), 0, prefix);
        }
    }
    else
    {
        std::cout << "(empty)" << std::endl;
    }
    std::cout << "====================" << std::endl;
}

void	printServers(const std::vector<Server>& servers)
{
	std::cout << "\n========== SERVERS ==========" << std::endl;

	if (servers.empty())
	{
		std::cout << "(no servers created)" << std::endl;
		std::cout << "=============================" << std::endl;
		return;
	}

	for (size_t i = 0; i < servers.size(); ++i)
	{
		const Server& server = servers[i];

		std::cout << "Server " << (i + 1) << ":" << std::endl;
		std::cout << "  Server Name: " << server.getServerName() << std::endl;

		std::cout << "  Listen Addresses & Ports:" << std::endl;
		const std::map<std::string, std::string>& addressesPorts = server.getAddressesAndPorts();
		for (const auto& pair : addressesPorts)
		{
			std::cout << "    " << pair.first << ":" << pair.second << std::endl;
		}

		std::cout << "  Max Body Size: " << server.getMaxBodySize() << " bytes" << std::endl;

		std::cout << "  Error Pages:" << std::endl;
		const std::map<int, std::string>& errors = server.getErrors();
		if (errors.empty())
		{
			std::cout << "    (none)" << std::endl;
		}
		else
		{
			for (const auto& error : errors)
			{
				std::cout << "    " << error.first << " -> " << error.second << std::endl;
			}
		}

		std::cout << "  Locations:" << std::endl;
		const std::vector<Location>& locations = server.getLocations();
		if (locations.empty())
		{
			std::cout << "    (none)" << std::endl;
		}
		else
		{
			for (size_t j = 0; j < locations.size(); ++j)
			{
				const Location& location = locations[j];
				std::cout << "    Location " << (j + 1) << ":" << std::endl;
				std::cout << "      Path: " << location.getPath() << std::endl;
				std::cout << "      Root: " << (location.getRoot().empty() ? "(default)" : location.getRoot()) << std::endl;
				std::cout << "      Index: " << (location.getIndex().empty() ? "(default)" : location.getIndex()) << std::endl;
				std::cout << "      Autoindex: " << (location.getAutoindex() ? "on" : "off") << std::endl;
				std::cout << "      Allowed Methods: ";

				std::vector<std::string> methods;
				if (location.getGetMethod()) methods.push_back("GET");
				if (location.getPostMethod()) methods.push_back("POST");
				if (location.getDeleteMethod()) methods.push_back("DELETE");

				if (methods.empty())
				{
					std::cout << "(none)";
				}
				else
				{
					for (size_t k = 0; k < methods.size(); ++k)
					{
						std::cout << methods[k];
						if (k < methods.size() - 1)
							std::cout << ", ";
					}
				}
				std::cout << std::endl;
			}
		}

		if (i < servers.size() - 1)
			std::cout << std::endl;
	}

	std::cout << "=============================" << std::endl;
}
