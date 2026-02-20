/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Headers.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/31 12:40:10 by rkaras        #+#    #+#                 */
/*   Updated: 2026/02/20 13:41:38 by rkaras        ########   odam.nl         */
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
		
	if (rawKey == "host")
	{
		//duplicate Host header
		if (req.headers.find("host") != req.headers.end())
		throw HttpException(400, "Multiple host headers");
		
		//empty Host header
		if (value.empty())
		throw HttpException(400, "Invalid Host header");

		//:port validation
		size_t colonPos = value.find(':');
		if (colonPos != std::string::npos)
		{
			std::string port = value.substr(colonPos + 1);
			if (port.empty())
				throw HttpException(400, "Invalid Host header");
			for (size_t i = 0; i < port.length(); i++)
			{
				if (!std::isdigit(port[i]))
					throw HttpException(400, "Invalid Host header");
			}
		}
	}

	if (rawKey == "content-length")
	{
		if (req.headers.find("content-length") != req.headers.end())
			throw HttpException(400, "Multiple Content-Length headers");
	}

	if (rawKey == "transfer-encoding")
	{
		if (req.headers.find("transfer-encoding") != req.headers.end())
			throw HttpException(400, "Multiple Transfer-Encoding headers");
	}
	
	req.headers[rawKey] = value; 
}