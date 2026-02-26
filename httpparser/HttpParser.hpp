/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpParser.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/25 15:36:03 by rkaras        #+#    #+#                 */
/*   Updated: 2026/02/24 17:23:14 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#pragma once

#include "../Client.hpp"
#include "../responder/Responder.hpp"
#include "HttpException.hpp"

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <fstream>
#include <algorithm>


struct HttpRequest
{
	std::string method;
	std::string uri;								//path only
	std::string query;								//query string after '?'
	std::map<std::string, std::string> queryMap;	//parsed query parameters
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;
};

struct ConnectionContext
{
	std::string buffer;
	unsigned long long maxBodySize;
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
		void								parseRequestLine(const std::string &line, HttpRequest &req);
		void								parseHeaderLine(const std::string &line, HttpRequest &req);
		size_t								bodyLength(const HttpRequest &req, const unsigned long long maxBodySize);
		bool								readLine(const char *buf, size_t length, size_t &pos, std::string &out);
		bool								isChunked(const HttpRequest &req);
		ParseStatus							parseChunkedBody(ConnectionContext &ctx, size_t bodyStart);
		std::map<std::string, std::string>	parseQueryString(const std::string &query);
		
		public:
		HttpParser() = default;
		HttpParser(const HttpParser& other) = delete;
		HttpParser&	operator=(const HttpParser& other) = delete;
		~HttpParser() = default;
		
		ParseStatus initParser(Client &client);
		ParseStatus	parseRequest(ConnectionContext &ctx);
};

#endif /* !HTTPPARSER_H */