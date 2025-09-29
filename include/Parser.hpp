#pragma once

#include "Configuration.hpp"
#include "Lexer.hpp"

class ParseError : public std::runtime_error
	{
		private:
			size_t	_line;
			size_t	_column;

		public:
			ParseError(const std::string& message, size_t line, size_t column);

			size_t	getLine() const { return _line; }
			size_t	getColumn() const { return _column; }

			const char *what() const noexcept override;
	};

class	Parser
{
	private:
		std::vector<Token>	_tokens;
		size_t				_currentIndex;

		const Token&	currentToken() const;
		void			advance();
		const Token&	peekToken(size_t offset = 1) const;	// Default = 1
		bool			isAtEnd();
		void			expectToken(TokenType type, const std::string& errorMessage);

		std::unique_ptr<ASTNode>			parseDirective();
		std::unique_ptr<SimpleDirective>	parseSimpleDirective();
		std::unique_ptr<BlockDirective>		parseBlockDirective();
		std::vector<std::string>			parseParameters();


	public:
		Parser() = delete;
		Parser(std::vector<Token>& tokens);

		std::unique_ptr<ConfigFile>	parse();

};

