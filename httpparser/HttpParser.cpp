/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpParser.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/25 15:36:17 by rkaras        #+#    #+#                 */
/*   Updated: 2026/02/18 19:50:00 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"
// #include "../server/Client.hpp"

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
	while (readLine(buf, length, pos, line) && line.empty())
	{
		//do nothing, just skip the possible leading CRLFs
	}
	if (line.empty())
		return ParseStatus::ERROR;
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
	{
		std::cerr << "Warning: both Transfer-Encoding and Content-Length present; ignoring Content-Length. \n";
		return (parseChunkedBody(ctx, bodyStart));
	}
	else if (hasChunked)
		return (parseChunkedBody(ctx, bodyStart));
	else if (hasContentLength)
	{
		size_t expectedBody = bodyLength(ctx.request, ctx.maxBodySize);
		if (availableBody < expectedBody)
			return ParseStatus::INCOMPLETE;

		if (availableBody > expectedBody)
			throw HttpException(400, "Body larger than Content-Length");
			
		ctx.request.body = ctx.buffer.substr(bodyStart, expectedBody);
		ctx.buffer.erase(0, bodyStart + expectedBody);
		ctx.headerEnd = std::string::npos;
		return (ParseStatus::COMPLETE);
	}
	else
	{
		ctx.request.body.clear();
		ctx.buffer.erase(0, bodyStart);
		ctx.headerEnd = std::string::npos;
		return ParseStatus::COMPLETE;
	}
	
}

ParseStatus HttpParser::initParser(Client &client)
{
	ConnectionContext ctx;
	ctx.buffer = client._buf;
	ctx.maxBodySize = client._maxBodySize;

	Responder responder;
	
	try
	{
		ParseStatus status = parseRequest(ctx);
		
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
				
			client.response = responder.buildResponse(ctx.request, client._alive);
			
			client._buf.clear();
		}

		return (status);
	}
	catch (HttpException &e)
	{
		client.response = responder.buildErrorResponse(e.getStatus(), false);	
			
		client._alive = false;
		client._buf.clear();
		return ParseStatus::ERROR;
	}
}