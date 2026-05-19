/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vknape <vknape@student.codam.nl>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 15:36:03 by rkaras            #+#    #+#             */
/*   Updated: 2025/11/27 11:57:41 by vknape           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPARSER_H
#define HTTPPARSER_H

// #include "../server/Client.hpp"
// #include "../src/Client.hpp"

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <fstream>
#include <algorithm>

class Client; // Forward declaration is enough, and we can include the header in the .cpp file that uses it (so we don't include it in the others that don't use it).

struct HttpRequest
{
	std::string method;
	std::string uri;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;
};

struct ConnectionContext
{
	std::string buffer;
	size_t headerEnd = std::string::npos;
	HttpRequest request;
};

enum ParseStatus
{
	INCOMPLETE,
	COMPLETE,
	ERROR
};

class HttpParser
{
	private:
		void		parseRequestLine(const std::string &line, HttpRequest &req);
		void		parseHeaderLine(const std::string &line, HttpRequest &req);
		size_t		bodyLength(const HttpRequest &req);
		bool		readLine(const char *buf, size_t length, size_t &pos, std::string &out);
		bool		isChunked(const HttpRequest &req);
		ParseStatus	parseChunkedBody(ConnectionContext &ctx, size_t bodyStart);
		
		public:
		HttpParser() = default;
		HttpParser(const HttpParser& other) = delete;
		HttpParser&	operator=(const HttpParser& other) = delete;
		~HttpParser() = default;
		
		// void		appendData(ConnectionContext &ctx, const char *data, size_t len);
		ParseStatus initParser(Client &client);
		ParseStatus	parseRequest(ConnectionContext &ctx);
};

#endif /* !HTTPPARSER_H */