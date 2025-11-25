#pragma once

#include "../include/Configuration.hpp"

//	---------- PRINT UTILITIES ---------- 

inline const char*		tokenTypeToString(TokenType type);
inline std::ostream&	operator<<(std::ostream& os, const Token& token);
void					printTokensList(const std::vector<Token>& tokenList);
void					printIndent(int indent, const std::string& prefix = "");
void					printSimpleDirective(const Directive* directive, int indent, const std::string& prefix);
void					printBlockDirective(const Directive* directive, int indent, const std::string& prefix);
void					printASTNode(const Directive* node, int indent = 0, const std::string& prefix = "");
void					printAST(const std::unique_ptr<ConfigFile>& config);
void					printServers(const std::vector<Server>& servers);

//