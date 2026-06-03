/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Body.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/31 12:41:30 by rkaras        #+#    #+#                 */
/*   Updated: 2026/05/20 10:55:50 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"
#include <vector>

size_t	HttpParser::bodyLength(const HttpRequest &req)
{
	std::map<std::string, std::string>::const_iterator it = req.headers.find("content-length");
	if (it == req.headers.end())
		return 0;

	const std::string &value = it->second;
	if (value.empty())
		throw HttpException(400, "Invalid Content-Length: non-digit characters");

	for (size_t i = 0; i < value.length(); i++)
	{
		if (!std::isdigit(value[i]))
			throw HttpException(400, "Invalid Content-Length");
	}

	unsigned long long len = 0;

	try {
		len = std::stoull(value);
	} catch (const std::out_of_range &) {
		throw HttpException(400, "Invalid Content-Length: overflow");
	} catch (...) {
		throw HttpException(400, "Invalid Content-Length");
	}

	return static_cast<size_t>(len);
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

	//see if there's optional Trailer header to know which trailer fields to accept
	std::vector<std::string> allowedTrailers;
	std::map<std::string, std::string>::iterator itTrailer = ctx.request.headers.find("trailer");
	if (itTrailer != ctx.request.headers.end())
	{
		std::string value = itTrailer->second;
		std::istringstream ss(value);
		std::string field;
		while (std::getline(ss, field, ','))
		{
			field.erase(0, field.find_first_not_of(" \t"));
			field.erase(field.find_last_not_of(" \t") + 1);
			std::transform(field.begin(), field.end(), field.begin(), ::tolower);
			if (!field.empty())
				allowedTrailers.push_back(field);
		}
	}
	
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

		unsigned long chunkSize = 0;
		try {
			chunkSize = std::stoul(line, nullptr, 16);
		} catch (...) {
			throw HttpException(400, "Invalid chunk size");
		}

		if (chunkSize == 0) //last chunk
		{
			//reading the trailer
			while (true)
			{
				std::string trailerLine;
				if (!readLine(buffer, length, pos, trailerLine))
					return ParseStatus::INCOMPLETE;
				if (trailerLine.empty())
					break;
					
				size_t colon = trailerLine.find(':');
				if (colon != std::string::npos)
				{
					std::string key = trailerLine.substr(0, colon);
					std::string value = trailerLine.substr(colon + 1);
					key.erase(0, key.find_first_not_of(" \t" ));
					key.erase(key.find_last_not_of(" \t") + 1);
					std::transform(key.begin(), key.end(), key.begin(), ::tolower);
					
					if (std::find(allowedTrailers.begin(), allowedTrailers.end(), key) != allowedTrailers.end())
					{
						value.erase(0, value.find_first_not_of(" \t"));
						value.erase(value.find_last_not_of(" \t") + 1);
						ctx.request.headers[key] = value;
					}

				}
			}
			
			ctx.request.body = decoded;

			//set Content-Length and remove "chunked"
			ctx.request.headers["content-length"] = std::to_string(decoded.size());
			std::map<std::string, std::string>::iterator it = ctx.request.headers.find("transfer-encoding");
			if (it != ctx.request.headers.end() && it->second.find("chunked") != std::string::npos)
			{
				std::string value = it->second;
				size_t pos = value.find("chunked");
				value.erase(pos, 7);
				value.erase(0, value.find_first_not_of(", "));
				value.erase(value.find_last_not_of(", ")+ 1);
				if (value.empty())
					ctx.request.headers.erase(it);
				else
					it->second = value;
			}
			
			ctx.buffer.erase(0, pos);
			ctx.headerEnd = std::string::npos;
			return (ParseStatus::COMPLETE);
		}

		// Chunked has no Content-Length, so cap the decoded body as it grows.
		// Checked against the announced chunk size, before buffering it.
		if (decoded.size() + chunkSize > MAX_REQUEST_BODY_SIZE)
			throw HttpException(413, "Payload too large");

		if (pos + chunkSize + 2 > length)
			return (ParseStatus::INCOMPLETE);

		decoded.append(&buffer[pos], chunkSize);
		pos += chunkSize;

		//CRLF check after chunk data
		if (pos + 1 >= length || buffer[pos] != '\r' || buffer[pos + 1] != '\n')
			throw HttpException(400, "Malformed chunked encoding");
		
		pos += 2;

	}
}