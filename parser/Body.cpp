/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Body.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/31 12:41:30 by rkaras        #+#    #+#                 */
/*   Updated: 2025/10/31 12:42:39 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"

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

		unsigned long chunkSize = 0;
		try {
			chunkSize = std::stoul(line, nullptr, 16);
		} catch (...) {
			throw std::runtime_error("Invalid chunk size");
		}

		// std::cout << "Chunk size (hex): " << line << " =>" << chunkSize << std::endl;

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

		std::cout << decoded << std::endl;

		if (pos + 1 >= length || buffer[pos] != '\r' || buffer[pos + 1] != '\n')
			throw std::runtime_error("Missing CRLF after chunk/data");
		
		pos += 2;

		// std::cout << "Decoded so far: '" << decoded << "'" << std::endl;
	}
}