/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpParser.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/25 15:36:03 by rkaras        #+#    #+#                 */
/*   Updated: 2026/06/03 16:04:03 by lprieri       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#pragma once

#include "HttpException.hpp"

#include <cstddef>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <fstream>
#include <algorithm>

// Hard cap on a request body, enforced by the parser before buffering it.

# define MAX_REQUEST_BODY_SIZE 10485760 // 10 MB

class Client;

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
		size_t								bodyLength(const HttpRequest &req);
		bool								readLine(const char *buf, size_t length, size_t &pos, std::string &out);
		bool								isChunked(const HttpRequest &req);
		ParseStatus							parseChunkedBody(ConnectionContext &ctx, size_t bodyStart);
		std::map<std::string, std::string>	parseQueryString(const std::string &query);

	public:
		HttpParser() = default;
		HttpParser(const HttpParser& other) = delete;
		HttpParser&	operator=(const HttpParser& other) = delete;
		~HttpParser() = default;

		ParseStatus	initParser(Client &client);
		ParseStatus	parseRequest(ConnectionContext &ctx);
};

#endif /* !HTTPPARSER_H */
