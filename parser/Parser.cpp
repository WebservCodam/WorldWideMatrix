#include "../include/Parser.hpp"

ParseError::ParseError(const std::string& message, size_t line, size_t column) :
std::runtime_error("Parse error at line " + std::to_string(line)
					+ ", column " + std::to_string(column) + ": " + message),
					_line(line), _column(column) {}  // Initializer list

const char* ParseError::what() const noexcept {
	return std::runtime_error::what();
}

// Very basic for now.
Parser::Parser(std::vector<Token>& tokens)
{
	// Add checks for when the vector list is invalid.
	this->_tokens = tokens;
	this->_currentIndex = 0;
}

const	Token&	Parser::currentToken() const
{
	if (_currentIndex >= _tokens.size())
		return (_tokens.back());
	return _tokens[_currentIndex];
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
		return _tokens.back();	// Return EOF token if at the end
	return _tokens[peekingPos];
}

bool	Parser::isAtEnd()
{
	return (_currentIndex >= _tokens.size() || currentToken().type == END_OF_FILE);
}

void	Parser::expectToken(TokenType type, const std::string& errorMessage)
{
	if (currentToken().type != type)
	{
		throw ParseError(errorMessage + ", got '" + currentToken().value + "'",
						currentToken().line, currentToken().column);
	}
}

std::unique_ptr<ConfigFile>	Parser::parse()
{
	std::unique_ptr<ConfigFile>	config(new ConfigFile);
	
	if (!_tokens.empty())
	{
		config->line = _tokens[0].line;
		config->column = _tokens[0].column;
	}

	while (!isAtEnd())
	{
		try
		{
			std::unique_ptr<ASTNode>	directive = parseDirective();
			if (directive)
				config->directives.push_back(std::move(directive));	// Move transfers ownership from the local directive to the config->directives vector. The local directive becomes nullptr.
		}
		catch (const ParseError& e)
		{
			throw ; // Figure this out
		}
	}
	
	return (config);
}

std::unique_ptr<ASTNode>	Parser::parseDirective()
{
	if (currentToken().type != WORD)
	{
		throw ParseError("Expected directive name",
						currentToken().line, currentToken().column);
	}

	//	Skip all the words and numbers
	size_t	lookAhead = 1;
	while (peekToken(lookAhead).type == WORD
		|| peekToken(lookAhead).type == NUMBER
		|| peekToken(lookAhead).type == EQUALS
		|| peekToken(lookAhead).type == STRING
		|| peekToken(lookAhead).type == LBRACKET
		|| peekToken(lookAhead).type == RBRACKET
		|| peekToken(lookAhead).type == COMMA) // Would return EOF if at end
		lookAhead++;

	//	Determine the type of directive
	// if (peekToken(lookAhead).type == EQUALS)
	// 	return (parseExactMatchDirective());	//Usually in locations, but can also appear in errors, and can be either simple or blocks
	if (peekToken(lookAhead).type == SEMICOLON)
		return (parseSimpleDirective());
	else if (peekToken(lookAhead).type == LBRACE)
		return (parseBlockDirective());
	else
	{
		throw ParseError("Expected ';' or '{' after directive parameters",
						peekToken(lookAhead).line, peekToken(lookAhead).column);
	}
}

// std::unique_ptr<ASTNode>	Parser::initializeDirective() // Can't implement this due to having a simple or block directive. Review polymorphism.
// {
// 	std::unique_ptr<ASTNode>	directive(new ASTNode());

// 	directive->line	= currentToken().line;
// 	directive->column = currentToken().column;

// 	return (directive);
// }

// std::unique_ptr<ASTNode>	Parser::parseExactMatchDirective()
// {
// 	std::unique_ptr<ASTNode>	directive = initializeDirective();

// 	return (directive);
// }

std::unique_ptr<SimpleDirective>	Parser::parseSimpleDirective()
{
	std::unique_ptr<SimpleDirective>	directive(new SimpleDirective());

	directive->line	= currentToken().line;
	directive->column = currentToken().column;

	expectToken(WORD, "Expected directive name");
	directive->name = currentToken().value;
	advance();

	size_t	lookAhead = 1;
	while (peekToken(lookAhead).type != SEMICOLON && peekToken(lookAhead).type != EQUALS)
		lookAhead++;

	

	directive->parameters = parseParameters();

	expectToken(SEMICOLON, "Expected ';' after simple directive");
	advance();

	return (directive);
}

std::unique_ptr<BlockDirective>	Parser::parseBlockDirective()
{
	std::unique_ptr<BlockDirective>	directive(new BlockDirective);

	directive->line	= currentToken().line;
	directive->column	= currentToken().column;

	expectToken(WORD, "Expected directive name");
	directive->name = currentToken().value;
	advance();

	size_t	lookAhead = 1;
	while (peekToken(lookAhead).type != SEMICOLON && peekToken(lookAhead).type != EQUALS)
		lookAhead++;

	directive->parameters = parseParameters();

	expectToken(LBRACE, "Expected '{' to start block");
	advance();

	while (!isAtEnd() && currentToken().type != RBRACE)
	{
		std::unique_ptr<ASTNode>	child = parseDirective();
		if (child)
			directive->children.push_back(std::move(child));
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
		|| currentToken().type == LBRACKET
		|| currentToken().type == COMMA
		|| currentToken().type == RBRACKET)
	{
		parameters.push_back(currentToken().value);
		advance();
	}

	return (parameters);
}
