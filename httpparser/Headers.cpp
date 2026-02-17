/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Headers.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/31 12:40:10 by rkaras        #+#    #+#                 */
/*   Updated: 2026/02/17 16:10:32 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"

static bool isTchar(char c)
{
	return	std::isalnum(static_cast<unsigned char>(c)) ||
			c == '!' || c == '#' || c == '$' || c == '%' ||
			c == '&' || c == '\'' || c == '*' || c == '+' ||
			c == '-' || c == '.' || c == '^' || c == '_' ||
			c == '`' || c == '|' || c == '~';
}

void	HttpParser::parseHeaderLine(const std::string &line, HttpRequest &req)
{
	size_t colon = line.find(':');
	if (colon == std::string::npos)
		throw HttpException(400, "Malformed header: missing ':'");

	std::string rawKey = line.substr(0, colon);
	
	if (!rawKey.empty() && (rawKey.back() == ' ' || rawKey.back() == '\t'))
		throw HttpException(400, "Malformed header: whitespace before colon");
	
	rawKey.erase(0, rawKey.find_first_not_of(" \t"));
	
	if (rawKey.empty())
		throw HttpException(400, "Malformed header: empty field name");
	
	for (char c : rawKey)
	{
		if (!isTchar(c))
			throw HttpException(400, "Malformed header: invalid field name");
	}
	
	for (char &c : rawKey)
		c = std::tolower(c);
	
	std::string value = line.substr(colon + 1);
	value.erase(0, value.find_first_not_of(" \t"));
	value.erase(value.find_last_not_of(" \t") + 1);

	req.headers[rawKey] = value;
}