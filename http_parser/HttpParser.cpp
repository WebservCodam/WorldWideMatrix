/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpParser.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/25 15:36:17 by rkaras        #+#    #+#                 */
/*   Updated: 2025/10/01 18:38:18 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"


void HttpParser::appendData(ConnectionContext &ctx, const char *data, size_t len)
{
	ctx.buffer.append(data, len);
}

void HttpParser::parseRequestLine(const std::string &line, HttpRequest &req)
{
	std::istringstream requestLineStream(line);
	if (!(requestLineStream >> req.method >> req.uri >> req.version))
		throw std::runtime_error("Malformed request line");
}

void HttpParser::parseHeaderLine(const std::string &line, HttpRequest &req)
{
	size_t colon = line.find(':');
	if (colon == std::string::npos)
		throw std::runtime_error("Malformed header: missing ':'");

	std::string key = line.substr(0, colon);
	std::string value = line.substr(colon + 1);
	
	key.erase(0, key.find_first_not_of(" "));
	key.erase(key.find_last_not_of(" ") + 1);
	value.erase(0, value.find_first_not_of(" "));
	value.erase(value.find_last_not_of(" ") + 1);

	req.headers[key] = value;
}

bool	HttpParser::expectsBody(const HttpRequest &req)
{
	if (req.method == "POST")
		return (true);
	if (req.method == "DELETE" && req.headers.count("Content-Length"))
		return (true);
	return (false);
}

size_t	HttpParser::bodyLength(const HttpRequest &req)
{
	std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Length");
	if (it == req.headers.end())
		return 0;
	
	try {
		size_t len = std::stoul(it->second);
		return (len);
	} catch (const std::exception &e) {
		throw std::runtime_error("Invalid Content-Lenght header");
	}
}

ParseStatus HttpParser::parseRequest(ConnectionContext &ctx)
{
	ctx.headerEnd = ctx.buffer.find("\r\n\r\n");
	if (ctx.headerEnd == std::string::npos)
		return ParseStatus::INCOMPLETE;

	std::string headerPart = ctx.buffer.substr(0, ctx.headerEnd);
	std::istringstream stream(headerPart);
	std::string line;

	/* request line */
	if (!std::getline(stream, line))
		return ParseStatus::ERROR;
		
	if (!line.empty() && line.back() == '\r')
		line.pop_back();

	parseRequestLine(line, ctx.request);

	/* headers */
	while (std::getline(stream, line))
	{
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		if (line.empty())
			continue ;
		
		parseHeaderLine(line, ctx.request);
	}

	size_t expectedBody = 0;
	if (expectsBody(ctx.request) == true)
		expectedBody = bodyLength(ctx.request);

	/* body if exists */
	size_t bodyStart = ctx.headerEnd + 4;
	if (bodyStart < ctx.buffer.size())
		ctx.request.body = ctx.buffer.substr(bodyStart);
	
	return ParseStatus::COMPLETE;
}
