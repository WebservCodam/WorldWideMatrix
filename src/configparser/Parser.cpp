#include "Configuration.hpp"

// Very basic for now.
Parser::Parser(const std::string& input) : _input(input), _currentIndex(0) {}

const	Token&	Parser::currentToken() const
{
	if (_currentIndex >= _tokens.size())
		return (_tokens.back());
	return (_tokens[_currentIndex]);
}

void	Parser::advance()
{
	if (_currentIndex < _tokens.size() - 1)
		_currentIndex++;
}

const Token&	Parser::peekToken(size_t offset) const
{
	size_t	peekingPos = _currentIndex + offset;

	if (peekingPos >= _tokens.size())
		return (_tokens.back());	// Return EOF token if at the end
	return (_tokens[peekingPos]);
}

bool	Parser::isAtEnd()
{
	return (_currentIndex >= _tokens.size() || currentToken().type == END_OF_FILE);
}

void	Parser::expectToken(TokenType type, const std::string& errorMessage)
{
	if (currentToken().type != type)
	{
		throw ConfigError::parsing(errorMessage + ", got '" + currentToken().value + "'",
									currentToken().line, currentToken().column);
	}
}

std::unique_ptr<ConfigFile>	Parser::parse()
{
	std::vector<std::unique_ptr<Directive>>	directives;

	this->_tokens = Lexer(_input).tokenize();
	if (this->_tokens.empty())
		throw ConfigError::initialization("Empty list of tokens.");

	while (!isAtEnd())
	{
		std::unique_ptr<Directive>	directive = parseDirective();
		directives.push_back(std::move(directive));
	}

	this->_configFile = std::make_unique<ConfigFile>(std::move(directives)); //Calls constructor of ConfigFile that takes a vector of directives as arg.

	validateSemantics();

	return (std::move(this->_configFile));
}

std::unique_ptr<Directive>	Parser::parseDirective()
{
	if (currentToken().type != WORD)
	{
		throw ConfigError::parsing("Expected directive name",
									currentToken().line, currentToken().column);
	}

	std::unique_ptr<Directive>	directive = initializeDirective();

	if (currentToken().type == SEMICOLON)
	{
		advance();
		return (directive);
	}
	else if (currentToken().type == LBRACE)
	{
		advance();
		while (!isAtEnd() && currentToken().type != RBRACE)
		{
			std::unique_ptr<Directive>	child = parseDirective();
			if (child)
			{
				child->setContext(directive->getName());
				child->setParent(directive.get());
				directive->addChild(std::move(child));
			}
		}
		expectToken(RBRACE, "Expected '}' to close block");
		advance();
		return (directive);
	}
	else
	{
		throw ConfigError::parsing("Expected ';' or '{' after directive parameters",
									currentToken().line, currentToken().column);
	}
}

std::unique_ptr<Directive>	Parser::initializeDirective()
{
	std::unique_ptr<Directive>	directive = std::make_unique<Directive>();

	directive->setLine(currentToken().line);
	directive->setColumn(currentToken().column);
	directive->setContext("main");

	expectToken(WORD, "Expected directive name");
	directive->setName(currentToken().value);
	advance();

	directive->setParameters(std::move(parseParameters()));

	return (directive);
}

std::vector<std::string>	Parser::parseParameters()
{
	std::vector<std::string>	parameters;
	size_t						startLine = currentToken().line;

	while ((currentToken().type == WORD
		|| currentToken().type == STRING
		|| currentToken().type == COMMA)
		&& currentToken().line == startLine)
	{
		parameters.push_back(currentToken().value);
		advance();
	}

	return (parameters);
}

void	Parser::checkDefaultErrorPages(std::vector<std::unique_ptr<Directive>>& directives)
{
	Directive*	root = nullptr;
	std::string	errorPagesRoot;
	std::string	uri40x;
	std::string uri50x;
	struct stat	st;

	for (auto it = directives.begin(); it != directives.end(); ++it)
	{
		if ((*it)->getName() == "root")
		{
			root = it->get();
			break;
		}
	}
	if (!root)
		errorPagesRoot = "./www/error_pages/";
	else
		errorPagesRoot = joinPath(root->getParameter(0), "error_pages");

	uri40x = joinPath(errorPagesRoot, "40x.html");
	uri50x = joinPath(errorPagesRoot, "50x.html");

	checkPath(uri40x, ErrorType::VALIDATOR, "40x error page: " + uri40x, false);
	checkPath(uri50x, ErrorType::VALIDATOR, "50x error page: " + uri50x, false);
}

void	Parser::validateSemantics()
{
	std::vector<std::unique_ptr<Directive>>&	directives = _configFile->getDirectives();
	
	// std::cout << "DEBUG: In validateSemantics" << std::endl;
	if (!_configFile->findDirective("server"))
		throw ConfigError::custom(ErrorType::PARSER, std::string("There was no server directive found in the configuration file."));
	for (std::unique_ptr<Directive>& directive : directives)
	{
		validateDirective(directive.get());
		// std::cout << "DEBUG: directive name: " << directive.get()->getName() << std::endl;
	}
	checkDefaultErrorPages(directives);
	return ;
}
