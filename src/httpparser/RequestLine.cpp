/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestLine.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/31 12:37:52 by rkaras        #+#    #+#                 */
/*   Updated: 2026/02/26 16:46:44 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"

// Recipients of an invalid request-line SHOULD respond with either a
// 400 (Bad Request) error or a 301 (Moved Permanently) redirect with
// the request-target properly encoded.

// HTTP does not place a predefined limit on the length of a
// request-line, as described in Section 2.5.  A server that receives a
// method longer than any that it implements SHOULD respond with a 501
// (Not Implemented) status code.  A server that receives a request-target longer than any URI it wishes to parse MUST respond
// with a 414 (URI Too Long) status code (see Section 6.5.12 of
// [RFC7231]).

// Various ad hoc limitations on request-line length are found in
// practice.  It is RECOMMENDED that all HTTP senders and recipients
// support, at a minimum, request-line lengths of 8000 octets.

std::map<std::string, std::string>	HttpParser::parseQueryString(const std::string &query)
{
	std::map<std::string, std::string> result;
	std::string key, value;
	std::istringstream stream(query);

	while (std::getline(stream, key,'&'))
	{
		size_t eqPos = key.find('=');
		if (eqPos != std::string::npos)
		{
			value = key.substr(eqPos + 1);
			key = key.substr(0, eqPos);
		}
		else
			value.clear();
		result[key] = value;
	}
	return result;
}

// Returns true if `uri` contains a ".." path segment (e.g. "/a/../b" or a
// trailing "/.."). Path resolution later only concatenates the URI onto the
// location's directory, so a ".." segment would let the request climb out of
// the web root ("/../../etc/passwd"); such requests are rejected outright.
static bool	hasDotDotSegment(const std::string &uri)
{
	size_t	pos = 0;

	while (pos < uri.size())
	{
		size_t		slash = uri.find('/', pos);
		std::string	segment = (slash == std::string::npos)
			? uri.substr(pos) : uri.substr(pos, slash - pos);

		if (segment == "..")
			return (true);
		if (slash == std::string::npos)
			break ;
		pos = slash + 1;
	}
	return (false);
}

void	HttpParser::parseRequestLine(const std::string &line, HttpRequest &req)
{
	if (line.length() > 8000) //octet = 1 byte = 1 char
		throw HttpException(414, "URI too long");
		
	//basic format and spacing check
	if (line.empty() || line[0] == ' ' || line.back() == ' ' ||line.find('\t') != std::string::npos)
		throw HttpException(400, "Malformed request line");
	
	if (line.find("  ") != std::string::npos)
		throw HttpException(400, "Malformed request line");
		
	size_t spaceCount = 0;
	for (size_t i = 0; i < line.size(); i++) {
		if (line[i] == ' ')
			spaceCount++;
	}
	if (spaceCount != 2)
		throw HttpException(400, "Malformed request line");
	
	size_t space1 = line.find(' ');
	size_t space2 = line.find(' ', space1 + 1);	
	
	if (space1 == 0 || space2 == std::string::npos)
		throw HttpException(400, "Malformed request line");
	
	req.method = line.substr(0, space1);
	std::string fullURI = line.substr(space1 + 1, space2 - space1 - 1);
	req.version = line.substr(space2 + 1);
	
	if (req.version != "HTTP/1.1")
		throw HttpException(505, "HTTP version not supported");

	if (req.method != "GET" && req.method != "POST" && req.method != "DELETE")
		throw HttpException(501, "Method not implemented");

	//splitting URI and query strings
	size_t qpos = fullURI.find('?');
	if (qpos != std::string::npos)
	{
		req.uri = fullURI.substr(0, qpos);
		req.query = fullURI.substr(qpos + 1);
		req.queryMap = parseQueryString(req.query);
	}
	else
	{
		req.uri = fullURI;
		req.query.clear();
	}

	if (hasDotDotSegment(req.uri))
		throw HttpException(400, "Path traversal is not allowed");
}
