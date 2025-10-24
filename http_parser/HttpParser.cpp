/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpParser.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/25 15:36:17 by rkaras        #+#    #+#                 */
/*   Updated: 2025/10/24 15:31:28 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"


void	HttpParser::appendData(ConnectionContext &ctx, const char *data, size_t len)
{
	ctx.buffer.append(data, len);
}

// The line provided to parseRequestLine() must already be extracted
// from the raw message (terminated by CRLF and ASCII-clean).
// It is therefore safe to process it as a std::string using C++ stream operators.

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
void	HttpParser::parseRequestLine(const std::string &line, HttpRequest &req)
{
	if (line.empty() || line[0] == ' ' || line.back() == ' ' ||line.find('\t') != std::string::npos)
		throw std::runtime_error("Malformed request line:\n please use this formatting:\n method SP request-target SP HTTP-version CRLF");
	
	if (line.find("  ") != std::string::npos)
		throw std::runtime_error("Malformed request line:\n please use this formatting:\n method SP request-target SP HTTP-version CRLF");
		
	size_t spaceCount = 0;
	for (size_t i = 0; i < line.size(); i++) {
		if (line[i] == ' ')
			spaceCount++;
	}
	if (spaceCount != 2)
		throw std::runtime_error("Malformed request line:\n please use this formatting:\n method SP request-target SP HTTP-version CRLF");
	
	size_t space1 = line.find(' ');
	size_t space2 = line.find(' ', space1 + 1);	
	
	if (space1 == 0 || space2 == std::string::npos)
		throw std::runtime_error("Malformed request line: incorrect space placement");
	
	req.method = line.substr(0, space1);
	req.uri = line.substr(space1 + 1, space2 - space1 - 1);
	req.version = line.substr(space2 + 1);
	
	if (req.version != "HTTP/1.1" && req.version != "HTTP/1.0")
		throw std::runtime_error("Unsupported HTTP version");

	if (req.method != "GET" && req.method != "POST" && req.method != "DELETE")
		throw std::runtime_error("Unsupported HTTP method");
}

static bool isTchar(char c)
{
	return	std::isalnum(static_cast<unsigned char>(c)) ||
			c == '!' || c == '#' || c == '$' || c == '%' ||
			c == '&' || c == '\'' || c == '*' || c == '+' ||
			c == '-' || c == '.' || c == '^' || c == '_' ||
			c == '`' || c == '|' || c == '~';
}

// No whitespace is allowed between the header field-name and colon.  In
//    the past, differences in the handling of such whitespace have led to
//    security vulnerabilities in request routing and response handling.  A
//    server MUST reject any received request message that contains
//    whitespace between a header field-name and colon with a response code
//    of 400 (Bad Request).
void	HttpParser::parseHeaderLine(const std::string &line, HttpRequest &req)
{
	size_t colon = line.find(':');
	if (colon == std::string::npos)
		throw std::runtime_error("Malformed header: missing ':'");

	std::string rawKey = line.substr(0, colon);
	
	if (!rawKey.empty() && (rawKey.back() == ' ' || rawKey.back() == '\t'))
		throw std::runtime_error("Malformed header: whitespace before colon not allowed");
	
	rawKey.erase(0, rawKey.find_first_not_of(" \t"));
	
	if (rawKey.empty())
		throw std::runtime_error("Malformed header: empty field name");
	
	for (char c : rawKey)
	{
		if (!isTchar(c))
			throw std::runtime_error("Malformed header: invalid character in field name");
	}
	
	for (char &c : rawKey)
		c = std::tolower(c);
	
	std::string value = line.substr(colon + 1);
	value.erase(0, value.find_first_not_of(" \t"));
	value.erase(value.find_last_not_of(" \t") + 1);

	req.headers[rawKey] = value;
}

// bool	HttpParser::expectsBody(const HttpRequest &req)
// {
// 	if (req.method == "POST")
// 		return (true);
// 	if (req.method == "DELETE" && req.headers.count("content-length"))
// 		return (true);
// 	return (false);
// }


size_t	HttpParser::bodyLength(const HttpRequest &req)
{
	std::map<std::string, std::string>::const_iterator it = req.headers.find("content-length");
	if (it == req.headers.end())
		return 0;

	const std::string &value = it->second;
	if (value.empty() || !std::all_of(value.begin(), value.end(), ::isdigit))
		throw std::runtime_error("Invalid Content-Length: non-digit characters");
	
	unsigned long long len = 0;
	try {
		len = std::stoull(value);
	} catch (const std::out_of_range &) {
		throw std::runtime_error("Invalid Content-Length: overflow");
	} catch (...) {
		throw std::runtime_error("Invalid Content-Length");
	}
	
	// check here if len > max body size from config file
	
	return static_cast<size_t>(len);
}

bool	HttpParser::readLine(const char *buf, size_t length, size_t &pos, std::string &out)
{
	out.clear();

	while (pos < length)
	{
		char c = buf[pos++];

		if (c == '\r')
		{
			if (pos < length && buf[pos] == '\n')
			{
				++pos;
				return (true);
			}
			throw std::runtime_error("Malformed line: CR not followed by LF");
		}
		out += c;
	}
	return (!out.empty());
}

bool	HttpParser::isChunked(const HttpRequest &req)
{
	std::map<std::string, std::string>::const_iterator it = req.headers.find("transfer-encoding");
	if (it == req.headers.end())
		return (false);

	std::string value = it->second;
	std::transform(value.begin(), value.end(), value.begin(), ::tolower);

	return (value.find("chunked") != std::string::npos);
}

ParseStatus	HttpParser::parseChunkedBody(ConnectionContext &ctx, size_t bodyStart)
{
	const char *buffer = ctx.buffer.data();
	size_t length = ctx.buffer.size();
	size_t pos = bodyStart;
	std::string decoded;

	while (true)
	{
		std::string line;
		if (!readLine(buffer, length, pos, line))
			return (ParseStatus::INCOMPLETE);

		if (line.empty())
			continue;

		size_t semicolon = line.find(';');
		if (semicolon != std::string::npos)
			line = line.substr(0, semicolon);

		size_t chunkSize = 0;
		try {
			chunkSize = std::stoul(line, nullptr, 16);
		} catch (...) {
			throw std::runtime_error("Invalid chunk size");
		}

		if (chunkSize == 0)
		{
			while (true)
			{
				std::string trailer;
				if (!readLine(buffer, length, pos, trailer))
					return ParseStatus::INCOMPLETE;
				if (trailer.empty())
					break;
			}
			ctx.request.body = decoded;
			ctx.buffer.erase(0, pos);
			ctx.headerEnd = std::string::npos;
			return (ParseStatus::COMPLETE);
		}

		if (pos + chunkSize + 2 > length)
			return (ParseStatus::INCOMPLETE);

		decoded.append(&buffer[pos], chunkSize);
		pos += chunkSize;

		if (buffer[pos] != '\r' || buffer[pos + 1] != '\n')
			throw std::runtime_error("Missing CRLF after chunk/data");
		
		pos += 2;
	}
}

ParseStatus	HttpParser::parseRequest(ConnectionContext &ctx)
{
	ctx.headerEnd = ctx.buffer.find("\r\n\r\n");
	if (ctx.headerEnd == std::string::npos)
		return ParseStatus::INCOMPLETE;

	const char *buf = ctx.buffer.data();
	size_t length = ctx.buffer.size();
	size_t pos = 0;
	std::string line;
	
	/* request line */
	if (!readLine(buf, length, pos, line))
		return ParseStatus::ERROR;
	parseRequestLine(line, ctx.request);

	/* headers */
	while (readLine(buf, length, pos, line))
	{
		if (line.empty())
			break ;
		if (line[0] == ' ' || line[0] == '\t')
			throw std::runtime_error("Malformed header: leading whitespace");
		parseHeaderLine(line, ctx.request);
	}

	/* body */
	// RFC says we're supposed to read the whole request if it has Content-length (or Transfer-encoding)
	// no matter the method. The body can be ignored outside of the parser if we decide it's not important.
	// Transfer-encoding was added in HTTP/1.1, so it also depends which one we're accepting?

	// we're doing 1.1
	// I should implement the chunked request as well

	// edge case - if there is no content length or no transfer encoding do we want to read and store the body or not?
	size_t bodyStart = ctx.headerEnd + 4;
	size_t availableBody = 0;
	
	if (ctx.buffer.size() > bodyStart)
		availableBody = ctx.buffer.size() - bodyStart;

	if (isChunked(ctx.request))
	{
		return (parseChunkedBody(ctx, bodyStart));
	}
	else
	{
		size_t expectedBody = bodyLength(ctx.request);
		if (availableBody < expectedBody)
			return ParseStatus::INCOMPLETE;

		//check if the available body is bigger than expected -> error
	
		ctx.request.body = ctx.buffer.substr(bodyStart, expectedBody);
		ctx.buffer.erase(0, bodyStart + expectedBody);
		ctx.headerEnd = std::string::npos;
		return (ParseStatus::COMPLETE);
	}
	
}
