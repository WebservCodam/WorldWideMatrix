/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/04 15:11:22 by rkaras        #+#    #+#                 */
/*   Updated: 2025/09/25 15:08:57 by rkaras        ########   odam.nl         */
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

HttpRequest Server::parseRequest(const std::string &buffer)
{
	HttpRequest request;

	std::string header = buffer.substr(0, _headerEnd);
	request.body = buffer.substr(_headerEnd + 4);

	std::istringstream stream(header);
	std::string line;

	// request line
	if (std::getline(stream, line))
	{
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
			
		std::istringstream reqLine(line);
		reqLine >> request.method >> request.uri >> request.version;
	}

	// header
	while (std::getline(stream, line))
	{
		if (!line.empty() && line.back() == '\r')
			line.pop_back();

		size_t colon = line.find(":");
		if (colon != std::string::npos)
		{
			std::string key = line.substr(0, colon);
			std::string value = line.substr(colon + 1);

			key.erase(0, key.find_first_not_of(" "));
			key.erase(key.find_last_not_of(" ") + 1);
			value.erase(0, value.find_first_not_of(" "));
			value.erase(value.find_last_not_of(" ") + 1);

			request.headers[key] = value;
		}
	}
	return (request);
}

void Server::accepter()
{
	struct sockaddr_in address = _socket->getAddress();
	int addrlen = sizeof(address);
	
	
	_new_socket = accept(_socket->getServerFd(), (struct sockaddr *)&address, (socklen_t *)&addrlen);
	
	char buf[1024];
	ssize_t bytesRead;
	
	while ((bytesRead = read(_new_socket, buf, sizeof(buf))) > 0)
	{
		_buffer.append(buf, bytesRead);
	}
}

void Server::handler()
{
	std::cout << "Method: " << _request.method << std::endl
		<< "URI: " << _request.uri << std::endl
		<< "Version: " << _request.version << std::endl;

	for (std::map<std::string, std::string>::iterator it = _request.headers.begin();
		it != _request.headers.end(); it++)
	{
		std::cout << it->first << ": " << it->second << std::endl;
	}
	
	if (!_request.body.empty())
		std::cout << "Body: " << _request.body << std::endl;
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

		ParseStatus status = parseRequestIncremental(_request)
		handler();
		responder();
		std::cout << "............. Done .............." << std::endl;
	}
}