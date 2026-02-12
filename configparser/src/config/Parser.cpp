#include "../../include/Configuration.hpp"

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
	std::unique_ptr<ConfigFile>				config;
	std::vector<std::unique_ptr<Directive>>	directives;

	this->_tokens = Lexer(_input).tokenize();
	if (this->_tokens.empty())
	{
		std::cerr << "Parsing error: Empty list of tokens" << std::endl;
		exit(EXIT_FAILURE);
	}

	while (!isAtEnd())
	{
		try
		{
			std::unique_ptr<Directive>	directive = parseDirective();
			if (directive)
				directives.push_back(std::move(directive));	// Move transfers ownership from the local directive to the config->directives vector. The local directive becomes nullptr.
		}
		catch (const ConfigError& e)
		{
			//throw ;  // Figure this out
			return (nullptr) ;
		}
	}

	config = std::make_unique<ConfigFile>(std::move(directives));
	return (config);
}

std::unique_ptr<Directive>	Parser::parseDirective()
{
	if (currentToken().type != WORD)
	{
		throw ConfigError::parsing("Expected directive name",
									currentToken().line, currentToken().column);
	}

	//	Skip all the words and numbers
	size_t	lookAhead = 1;
	while (peekToken(lookAhead).type != SEMICOLON && peekToken(lookAhead).type != LBRACE)
		lookAhead++;

	if (peekToken(lookAhead).type == SEMICOLON)
		return (parseSimpleDirective());
	else if (peekToken(lookAhead).type == LBRACE)
		return (parseBlockDirective());
	else
	{
		throw ConfigError::parsing("Expected ';' or '{' after directive parameters",
									peekToken(lookAhead).line, peekToken(lookAhead).column);
	}
}

std::unique_ptr<Directive>	Parser::initializeDirective()
{
	std::unique_ptr<Directive>	directive(new Directive());	//Add safeguards

	directive->setLine(currentToken().line);
	directive->setColumn(currentToken().column);
	directive->setContext("main");

	expectToken(WORD, "Expected directive name");
	directive->setName(currentToken().value);
	advance();

	directive->setParameters(std::move(parseParameters()));

	return (directive);
}

std::unique_ptr<Directive>	Parser::parseSimpleDirective()
{
	std::unique_ptr<Directive>	directive = initializeDirective();

	expectToken(SEMICOLON, "Expected ';' to close simple directive");
	advance();

	return (directive);
}

std::unique_ptr<Directive>	Parser::parseBlockDirective()
{
	std::unique_ptr<Directive>	directive = initializeDirective();

	expectToken(LBRACE, "Expected '{' to start block");
	advance();

	while (!isAtEnd() && currentToken().type != RBRACE)
	{
		std::unique_ptr<Directive>	child = parseDirective();
		if (child)
		{
			child->setContext(directive->getName());
			directive->addChild(std::move(child));
		}
	}

	expectToken(RBRACE, "Expected '}' to close block");
	advance();

	return (directive);
}

std::vector<std::string>	Parser::parseParameters()
{
	std::vector<std::string>	parameters;

	while (currentToken().type == WORD
		|| currentToken().type == NUMBER
		|| currentToken().type == STRING
		|| currentToken().type == COMMA)
	{
		parameters.push_back(currentToken().value);
		advance();
	}

	return (parameters);
}
