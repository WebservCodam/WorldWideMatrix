/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/04 15:11:22 by rkaras        #+#    #+#                 */
/*   Updated: 2025/09/12 15:46:35 by rkaras        ########   odam.nl         */
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
	
	char buf[1024];
	_buffer.clear();
	ssize_t bytesRead;
	
	while ((bytesRead = read(_new_socket, buf, sizeof(buf))) > 0)
	{
		_buffer.append(buf, bytesRead);
		if (_buffer.find("\r\n\r\n") != std::string::npos)
			break;
	}

}

void Server::handler()
{
	std::cout << _buffer << std::endl;
}

void Server::responder()
{
	std::string hello = "Hello from the server\n";
	std::string response =
		"HTTP/1.1 200 OK\r\n"
		"Content-type: text/html\r\n"
		"Content-Length: " + std::to_string(hello.size()) + "\r\n"
		"Connection: close\r\n"
		"\r\n" +
		hello;
		
	int sent = send(_new_socket, response.c_str(), response.size(), 0);
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