#pragma once

#include "Configuration.hpp"
#include "ConfigError.hpp"
#include "Lexer.hpp"

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

		std::unique_ptr<Directive>			initializeDirective();
		std::unique_ptr<Directive>			parseDirective();
		std::unique_ptr<Directive>			parseSimpleDirective();
		std::unique_ptr<Directive>			parseBlockDirective();
		std::vector<std::string>			parseParameters();

	public:
		Parser() = delete;
		Parser(Lexer& lexer);
		~Parser() = default;

		std::unique_ptr<ConfigFile>	parse();
};
