/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/04 15:09:11 by rkaras        #+#    #+#                 */
/*   Updated: 2025/09/25 15:01:34 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H
#define SERVER_H

#include "Socket.hpp"
#include "ListeningSocket.hpp"
#include <unistd.h>
#include <map>
#include <sstream>

enum ParseStatus {
	INCOMPLETE,
	COMPLETE,
	ERROR
};

struct HttpRequest
{
    std::string method;
    std::string uri;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
};

class Server
{
private:
	ListeningSocket *_socket;
	std::string _buffer;
	int _new_socket;
	size_t _headerEnd = std::string::npos;
	HttpRequest _request;
	
	void accepter();
	void handler();
	void responder();
	HttpRequest parseRequest(const std::string &buffer);

public:
	Server() = delete;
	Server(int domain, int service, int protocol, int port, u_long interface, int flag, int backlog);
	Server(const Server& other) = delete;
	Server& operator=(const Server& other) = delete;
	~Server() = default;

	void launch(); //infinite while loop that calls accepter, handler, responder

	ListeningSocket *getSocket();
};

#endif /* !SERVER_H */