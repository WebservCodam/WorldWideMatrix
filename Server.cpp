/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/04 15:11:22 by rkaras        #+#    #+#                 */
/*   Updated: 2025/09/04 16:07:32 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(int domain, int service, int protocol, int port, u_long interface, int flag, int backlog)
{
	_socket = new ListeningSocket(domain, service, protocol, port, interface, flag, backlog);
}

ListeningSocket *Server::getSocket()
{
	return _socket;
}

void Server::accepter()
{
	struct sockaddr_in address = _socket->getAddress();
	int addrlen = sizeof(address);
	_new_socket = accept(_socket->getServerFd(), (struct sockaddr *)&address, (socklen_t *)&addrlen);
	read(_new_socket, _buffer, 30000);
}

void Server::handler()
{
	std::cout << _buffer << std::endl;
}

void Server::responder()
{
	std::string hello = "Hello from the server\n";
	int sent = send(_new_socket, hello.c_str(), hello.size(), 0);
	_socket->connectionValid(sent);
	close(_new_socket);
}

void Server::launch()
{
	while (true)
	{
		std::cout << "............. Waiting .............." << std::endl;
		accepter();
		handler();
		responder();
		std::cout << "............. Done .............." << std::endl;
	}
}