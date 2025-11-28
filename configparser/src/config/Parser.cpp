#include "../../include/Parser.hpp"
#include "../../include/ConfigError.hpp"

// Very basic for now.
Parser::Parser(std::vector<Token>& tokens) : _tokens(tokens), _currentIndex(0) {}

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
	while (peekToken(lookAhead).type != SEMICOLON
		&& peekToken(lookAhead).type != LBRACE)
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

std::unique_ptr<Directive>	Parser::parseSimpleDirective()
{
	std::unique_ptr<Directive>	directive(new Directive());

	directive->setLine(currentToken().line);
	directive->setColumn(currentToken().column);

	expectToken(WORD, "Expected directive name");
	directive->setName(currentToken().value);
	advance();

	// If it already exists it was created by a block directive with a context. Otherwise it should be the main. (Double check that)
	if (directive->getContext().empty())
		directive->setContext("main");

	size_t	lookAhead = 1;
	while (peekToken(lookAhead).type != SEMICOLON && peekToken(lookAhead).type != EQUALS)
		lookAhead++;

	directive->setParameters(std::move(parseParameters()));

	expectToken(SEMICOLON, "Expected ';' after simple directive");
	advance();

	return (directive);
}

std::unique_ptr<Directive>	Parser::parseBlockDirective()
{
	std::unique_ptr<Directive>	directive(new Directive);

	directive->setLine(currentToken().line);
	directive->setColumn(currentToken().column);

	expectToken(WORD, "Expected directive name");
	directive->setName(currentToken().value);
	advance();

	if (directive->getContext().empty())
		directive->setContext("main");

	size_t	lookAhead = 1;
	while (peekToken(lookAhead).type != SEMICOLON && peekToken(lookAhead).type != EQUALS)
		lookAhead++;

	directive->setParameters(std::move(parseParameters()));

	expectToken(LBRACE, "Expected '{' to start block");
	advance();

	while (!isAtEnd() && currentToken().type != RBRACE)
	{
		std::unique_ptr<Directive>	child = parseDirective();
		child->setContext(directive->getName());
		if (child)
			directive->addChild(std::move(child));
	}

	expectToken(RBRACE, "Expected '}' to close block");
	advance();

	return (directive);
}

std::vector<std::string>	Parser::parseParameters()
{
	std::vector<std::string>	parameters;

	while (currentToken().type == EQUALS
		|| currentToken().type == WORD
		|| currentToken().type == NUMBER
		|| currentToken().type == STRING
		|| currentToken().type == LBRACKET
		|| currentToken().type == COMMA
		|| currentToken().type == RBRACKET)
	{
		parameters.push_back(currentToken().value);
		advance();
	}

	return (parameters);
}
