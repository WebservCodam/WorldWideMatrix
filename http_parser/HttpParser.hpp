/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpParser.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/25 15:36:03 by rkaras        #+#    #+#                 */
/*   Updated: 2025/10/03 15:52:08 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <fstream>


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

enum ParseStatus {
	INCOMPLETE,
	COMPLETE,
	ERROR
};

class HttpParser
{
	private:
		void	parseRequestLine(const std::string &line, HttpRequest &req);
		void	parseHeaderLine(const std::string &line, HttpRequest &req);
		bool	expectsBody(const HttpRequest &req);
		size_t	bodyLength(const HttpRequest &req);
	
	public:
		HttpParser() = default;
		HttpParser(const HttpParser& other) = delete;
		HttpParser& operator=(const HttpParser& other) = delete;
		~HttpParser() = default;

		void		appendData(ConnectionContext &ctx, const char *data, size_t len);
		ParseStatus	parseRequest(ConnectionContext &ctx);
};

#endif /* !HTTPPARSER_H */