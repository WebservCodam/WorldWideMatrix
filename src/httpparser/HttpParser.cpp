/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpParser.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/25 15:36:17 by rkaras        #+#    #+#                 */
/*   Updated: 2026/02/27 15:22:09 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"
#include "../Client.hpp"

ParseStatus	HttpParser::parseRequest(ConnectionContext &ctx)
{
	ctx.headerEnd = ctx.buffer.find("\r\n\r\n");
	if (ctx.headerEnd == std::string::npos)
		return ParseStatus::INCOMPLETE;
	
	if (ctx.headerEnd > 8000)
		throw HttpException(431, "Request headers too long");

	const char *buf = ctx.buffer.data();
	size_t length = ctx.buffer.size();
	size_t pos = 0;
	std::string line;	
	
	/* request line */
	while (readLine(buf, length, pos, line) && line.empty())
	{
		//do nothing, just skip the possible leading CRLFs
	}
	if (line.empty())
		throw HttpException(400, "Bad request");
	parseRequestLine(line, ctx.request);

	
	/* headers */
	while (readLine(buf, length, pos, line))
	{
		if (line.empty())
			break ;
		if (line[0] == ' ' || line[0] == '\t')
			throw HttpException(400, "Malformed header: leading whitespace");
		parseHeaderLine(line, ctx.request);
	}
	
	//enforcing the host header
	if (ctx.request.headers.find("host") == ctx.request.headers.end())
		throw HttpException(400, "Missing Host header");


	/* body */
	size_t bodyStart = ctx.headerEnd + 4;
	size_t availableBody = 0;
	
	if (ctx.buffer.size() > bodyStart)
		availableBody = ctx.buffer.size() - bodyStart;

	bool hasChunked = isChunked(ctx.request);
	bool hasContentLength = ctx.request.headers.count("content-length") > 0;

	if (hasChunked && hasContentLength)
		throw HttpException(400, "Conflicting length headers");
	
	if (hasChunked)
		return (parseChunkedBody(ctx, bodyStart));
	
	if (hasContentLength)
	{
		size_t expectedBody = bodyLength(ctx.request);

		// Reject oversized bodies from the Content-Length header alone, before
		// buffering a single byte of the body.
		if (expectedBody > MAX_REQUEST_BODY_SIZE)
			throw HttpException(413, "Payload too large");

		if (availableBody < expectedBody)
			return ParseStatus::INCOMPLETE;

		// Consume exactly Content-Length bytes as the body. Any bytes past that
		// belong to the next (pipelined) request, so they're left in the buffer
		// for the following parse round rather than treated as an oversized body.
		ctx.request.body = ctx.buffer.substr(bodyStart, expectedBody);
		ctx.buffer.erase(0, bodyStart + expectedBody);
		ctx.headerEnd = std::string::npos;
		return (ParseStatus::COMPLETE);
	}

	ctx.request.body.clear();
	ctx.buffer.erase(0, bodyStart);
	ctx.headerEnd = std::string::npos;
	return ParseStatus::COMPLETE;
}

ParseStatus HttpParser::initParser(Client &client)
{
	ParseStatus			status;
	ConnectionContext	ctx;

	ctx.buffer = client._buf;
	try
	{
		status = parseRequest(ctx);
	}
	catch(const HttpException& e)
	{
		std::cerr << "Parse exception: " << e.what() << std::endl;
		client._alive = false;
		client._buf.clear();
		client._response.status = e.getStatus();
		std::cout << "DEBUG in initParser - response status: " + std::to_string(e.getStatus()) << std::endl;
		return (ParseStatus::ERROR);
	}

	if (status == ParseStatus::COMPLETE)
	{
		std::map<std::string, std::string>::iterator it = ctx.request.headers.find("connection");
		if (it != ctx.request.headers.end())
		{
			std::string value = it->second;
			std::transform(value.begin(), value.end(), value.begin(), ::tolower);
			if (value == "keep-alive")
				client._alive = true;
		}
		else
			client._alive = false;

		client._buf = ctx.buffer;
	}
	else if (status == ParseStatus::ERROR)
	{
		client._alive = false;
		client._buf.clear();
	}

	client._request = ctx.request;

	return (status);
}
