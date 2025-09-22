/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/04 15:09:11 by rkaras        #+#    #+#                 */
/*   Updated: 2025/09/12 14:28:23 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H
#define SERVER_H

#include "Socket.hpp"
#include "ListeningSocket.hpp"
#include <unistd.h>
class Server
{
private:
	ListeningSocket *_socket;
	std::string _buffer;
	int _new_socket;
	
	void accepter();
	void handler();
	void responder();

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