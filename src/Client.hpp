/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Client.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/12 14:00:27 by vknape        #+#    #+#                 */
/*   Updated: 2026/05/20 10:45:13 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "utils.hpp"
#include "httpparser/HttpParser.hpp"

class	Server;

struct HttpResponse
{
	int									status;
	std::map<std::string, std::string>	headers;
	std::string							body;
	std::string							contentType = "text/html";	// Overridden per file by its MIME type.
};

class Client
{
	private:
	
	public:
		const int 			_clientFd;
		int					_listenFd;
		int 				_time;
		bool 				_alive = false;
		int					_readstate = 0;
		int					_parseready = 0;
		int 				_content_length = 0;
		std::string			_buf;
		HttpResponse		_response;
		HttpRequest			_request;
		bool				_parseFailed = false;

		Client(int fd);
		~Client();

		std::string	serializeResponse(); // This creates the ready to send response that'll be written to the socket.
		void		setListenFd(int listenFd);
		int			getListenFd() const;
		int			getFd() const;
		void		setTime();
		int			getTime();
		int			checkTime() const;
};
