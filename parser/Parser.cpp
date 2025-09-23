#include <Parser.hpp>

std::unique_ptr<ConfigFile> Parser::parse(const std::vector<Token>& tokens)
{
	tokens_ = &tokens;
	position_ = 0;

	std::unique_ptr<ConfigFile> config(new ConfigFile());
	if (!tokens.empty())
	{
		config->line = tokens[0].line;
		config->column = tokens[0].column;
	}

	while (!isAtEnd() && currentToken().type != END_OF_FILE)
	{
		try
		{
			auto directive = parseDirective();
			if (directive)
				config->directives.push_back(std::move(directive));
		}
		catch (const ParseError& e)
		{
			throw;
		}
	}

	return config;
}

const Token& Parser::currentToken() const
{
	if (position_ >= tokens_->size())
		return (*tokens_)[tokens_->size() - 1];
	return (*tokens_)[position_];
}

const Token& Parser::peekToken(size_t offset) const
{
	size_t peek_pos = position_ + offset;
	if (peek_pos >= tokens_->size())
		return (*tokens_)[tokens_->size() - 1];
	return (*tokens_)[peek_pos];
}

void Parser::advance()
{
	if (position_ < tokens_->size() - 1)
		position_++;
}

bool Parser::isAtEnd() const
{
	return position_ >= tokens_->size() || currentToken().type == END_OF_FILE;
}

void Parser::expectToken(TokenType type, const std::string& message)
{
	if (currentToken().type != type)
	{
		throw ParseError(message + ", got '" + currentToken().value + "'",
						currentToken().line, currentToken().column);
	}
}

std::unique_ptr<ASTNode> Parser::parseDirective()
{
	if (currentToken().type != WORD)
	{
		throw ParseError("Expected directive name",
						currentToken().line, currentToken().column);
	}

	size_t lookAhead = 1;
	while (peekToken(lookAhead).type == WORD || peekToken(lookAhead).type == NUMBER)
		lookAhead++;

	if (peekToken(lookAhead).type == SEMICOLON)
		return parseSimpleDirective();
	else if (peekToken(lookAhead).type == LBRACE)
		return parseBlockDirective();
	else
	{
		throw ParseError("Expected ';' or '{' after directive parameters",
						peekToken(lookAhead).line, peekToken(lookAhead).column);
	}
}

std::unique_ptr<SimpleDirective> Parser::parseSimpleDirective()
{
	std::unique_ptr<SimpleDirective> directive(new SimpleDirective());
	directive->line = currentToken().line;
	directive->column = currentToken().column;

	expectToken(WORD, "Expected directive name");
	directive->name = currentToken().value;
	advance();

	directive->parameters = parseParameters();

	expectToken(SEMICOLON, "Expected ';' after simple directive");
	advance();

	return directive;
}

std::unique_ptr<BlockDirective> Parser::parseBlockDirective()
{
	std::unique_ptr<BlockDirective> directive(new BlockDirective());
	directive->line = currentToken().line;
	directive->column = currentToken().column;

	expectToken(WORD, "Expected directive name");
	directive->name = currentToken().value;
	advance();

	directive->parameters = parseParameters();

	expectToken(LBRACE, "Expected '{' to start block");
	advance();

	while (!isAtEnd() && currentToken().type != RBRACE)
	{
		auto child = parseDirective();
		if (child)
			directive->children.push_back(std::move(child));
	}

	expectToken(RBRACE, "Expected '}' to close block");
	advance();

	return directive;
}

std::vector<std::string> Parser::parseParameters()
{
	std::vector<std::string> parameters;

	while (currentToken().type == WORD || currentToken().type == NUMBER)
	{
		parameters.push_back(currentToken().value);
		advance();
	}

	return parameters;
}