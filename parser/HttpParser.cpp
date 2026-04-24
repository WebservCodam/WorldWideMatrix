/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vknape <vknape@student.codam.nl>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 15:36:17 by rkaras            #+#    #+#             */
/*   Updated: 2025/11/21 11:31:24 by vknape           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"
#include "../src/Client.hpp"

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
		size_t expectedBody = bodyLength(ctx.request);
		if (availableBody < expectedBody)
			return ParseStatus::INCOMPLETE;

		if (availableBody > expectedBody)
			throw std::runtime_error("Body size exceeds Content-Length");
			
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
	ParseStatus			status;
	ConnectionContext	ctx;

	ctx.buffer = client._buf;
	try
	{
		status = parseRequest(ctx);
	}
	catch(const std::exception& e)
	{
		std::cerr << "Parse exception: " << e.what() << std::endl;
		client._alive = false;
		client._buf.clear();
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
			
		client._buf.clear();
	}
	else if (status == ParseStatus::ERROR)
	{
		client._alive = false;
		client._buf.clear();
	}

	client._request = ctx.request; // Adding this line, because don't we need to store it in the Client object?

	return (status);
}