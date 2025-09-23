#pragma once

#include <Configuration.hpp>
#include <Lexer.hpp>
#include <stdexcept>
#include <memory>

class ParseError : public std::runtime_error
{
	public:
		ParseError(const std::string& message, size_t line, size_t column)
			: std::runtime_error("Parse error at line " + std::to_string(line)
								+ ", column " + std::to_string(column) + ": " + message),
			  line_(line), column_(column) {}

		size_t getLine() const { return line_; }
		size_t getColumn() const { return column_; }

	private:
		size_t line_;
		size_t column_;
};

class Parser
{
	public:
		std::unique_ptr<ConfigFile> parse(const std::vector<Token>& tokens);

	private:
		const std::vector<Token>*	tokens_;
		size_t						position_;

		const Token& currentToken() const;
		const Token& peekToken(size_t offset = 1) const;
		void advance();
		bool isAtEnd() const;
		void expectToken(TokenType type, const std::string& message);

		std::unique_ptr<ASTNode> parseDirective();
		std::unique_ptr<SimpleDirective> parseSimpleDirective();
		std::unique_ptr<BlockDirective> parseBlockDirective();
		std::vector<std::string> parseParameters();
};