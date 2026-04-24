/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/01 10:59:15 by vknape        #+#    #+#                 */
/*   Updated: 2026/04/21 15:10:26 by lprieri       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"

Server::Server(int epfd): _epfd(epfd) {}

Server::~Server()
{
	close(_epfd);
}

void Server::initServer()
{
	for (const ServerConfig& server : _serverConfigs)
	{
		const std::vector<ListenDirective>&	addresses = server.getListenDirectives();

		for (const ListenDirective& addr : addresses)
		{
			// Are we treating each socket as a server of its own? A server should be able to handle multiple listening sockets.
			// Also, there's currently no separation of the different servers (corresponding to the different server configs).
			int	listenFd = createSocket(addr.address.c_str(), addr.port.c_str());
			_listenFds.push_back(listenFd);
		}
	}

	for (int fd: _listenFds)
	{
		addListeningSocketToEpoll(fd);
	}
}

/**
 * Adds the listening socket fd to epoll.
 */
void Server::addListeningSocketToEpoll(int listenFd)
{
	static struct epoll_event	event;

	event.data.fd = listenFd;
	event.events = EPOLLIN | EPOLLET | EPOLLPRI | EPOLLHUP;
	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, listenFd, &event) == -1)
		throw std::runtime_error("Server add to epoll failed");
}

void Server::startServer()
{
	Server		server(_epfd);
	epoll_event	events[EPOLL_NBR_EVENTS];
	int			numEvents = 0;
	int			connections = 0; // What is this going to be used for?

	while (true)
	{
		// printf("Connections made = %d\n", connections);
		// server.printBuffers();
		numEvents = epoll_wait(_epfd, events, EPOLL_NBR_EVENTS, 5000);
		// printf("Number of events waiting: %d\n", numEvents);
		if (numEvents == -1)
			throw std::runtime_error("Epoll_wait failed"); // 

		for (int i = 0; i < numEvents; i++)
		{
			// 
			if (std::find(_listenFds.begin(), _listenFds.end(), events[i].data.fd) != _listenFds.end())
			{
				server.connectNew(events[i].data.fd);
				connections++;
			}

			else if (events[i].events & EPOLLIN) // The & is similar to ==, but since more events can be stored in events that's why we're bitmasking rather than checking for equality.
			{
				server.connectIn(events[i].data.fd);
			}

			else if (events[i].events & EPOLLOUT)
			{
				server.connectOut(events[i].data.fd);
			}
		}
		server.checkHealth();
	}
}

void Server::closeClient(int fd)
{
	epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
	// _clientList.erase(fd);
}
void Server::addFdToClientList(int clientFd)
{
	_clientList.emplace(clientFd, clientFd);
}

void Server::connectNew(int listenFd)
{
	int	clientFd;

	while (true)
	{
		struct sockaddr_in	clientAddr;	// When we accept we configure this struct to match the address of the connecting peer.
		socklen_t			clientLen = sizeof(clientAddr);

		clientFd = accept(listenFd, (struct sockaddr*) &clientAddr, &clientLen);
		if (clientFd == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)	// Remember to remove these checks. Check the Edge Cases notes in Obsidian.
				return ;
			else
				throw std::runtime_error("Client accept failed");
		}

		setNonBlocking(clientFd); // Changed the function to void because in init we didn't check the return value, so now it directly throws an error from within the function.
		// {
		// 	// perror("Set flags failed");
		// 	// return (-1);
		// 	throw std::runtime_error("Client set flags failed");
		// }
		try
		{
			addFdToClientList(clientFd);
		}
		catch (const std::exception& e)
		{
			std::cout << "Failed to create and add Client object: " << e.what() << std::endl;
			throw ;
		}

		struct epoll_event	event;

		event.events = EPOLLIN; // We want to be notified when the fd is ready for reading. Removed the edge-triggered event, because it might be impossible to implement with the errno constraint.
		event.data.fd = clientFd;
		if (epoll_ctl(_epfd, EPOLL_CTL_ADD, clientFd, &event) == -1)
		{
			// perror("Epoll_ctl: add clientFd failed");
			// close(clientFd);
			
			throw std::runtime_error("Failed to add client to epoll");
		}
		// printf("here\n");
	}
}

void Server::connectIn(int clientFd)
{
	printf("\n----------------------------------------\nRead start\n");
	char	buffer[8192] = {0};
	ssize_t	count = 0; // Bytes read.

	count = recv(clientFd, buffer, sizeof(buffer), 0);
	_clientList.at(clientFd)._buf += buffer;
	// write(STDOUT_FILENO, buffer, count);
	
	std::cout << "DEBUG in connectIn: PRINTING BUFFER" << std::endl;
	std::cout << buffer << std::endl;
		
	// perror("Read error: ");
	// if (count == 0)
	// {
	// 	printf("Client %d disconnected\n", clientFd);
	// 	close(clientFd);
	// 	server.list.erase(clientFd);
	// 	return (0);
	// }
	// else if (count == -1)
	// {
	// 	// perror("Read failed");
	// 	// close(clientFd);
	// 	server.list.at(clientFd).readstate = count;
	// }

	parse(clientFd);

	std::cout << "DEBUG in connectIn" << std::endl;
	std::cout << "\n ---------- PARSE END ----------\n" << std::endl;

	struct epoll_event	event;

	event.events = EPOLLOUT | EPOLLET;
	event.data.fd = clientFd;
	if (epoll_ctl(_epfd, EPOLL_CTL_MOD, clientFd, &event) == -1) // When this function goes out of scope, what happens to event? It gets transfered? Does epoll_ctl copy it?
	{
		perror("Epoll_ctl: switch to EPOLLOUT failed");
		close(clientFd);
	}
	// printf("Read still open\n");
}

void Server::connectOut(int clientFd)
{
	printf("Writing now\n");
	std::ifstream		file_stream("index.html");
	std::stringstream	buffer;
	std::string			html;
	std::string			header;
	std::string			response;
	struct epoll_event	event;
	
	// if (!file_stream.is_open()) {
	//     return ""; // Return empty string if the file can't be opened
	// }
	
	buffer << file_stream.rdbuf();
	html =  buffer.str();
	file_stream.close();
	// const char* hello = "Hello from the server";
	// const char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
	header = "HTTP/1.1 200 OK\nContent-Type: text/html\n";
	header += "Content-Length: " + std::to_string(html.length()) + "\n\n";
	response = header + html;
	
	// write(clientFd, "hello", 5);
	// // close(clientFd);
	_clientList.at(clientFd)._buf.clear();
	write(clientFd, response.c_str(), response.length());
	if (_clientList.at(clientFd)._alive == false)
	{
		closeClient(clientFd);
		_clientList.erase(clientFd);
		return ;
	}
	event.events = EPOLLIN;
	event.data.fd = clientFd;
	if (epoll_ctl(_epfd, EPOLL_CTL_MOD, clientFd, &event) == -1)
	{
		perror("Epoll_ctl: switch to EPOLLIN failed");
		close(clientFd);
	}
}

void Server::checkHealth()
{
	auto	it = _clientList.begin();

	while (it != _clientList.end()) // Because the iterator is incremented within the loop, I changed this to a while loop.
	{
		std::cout << "DEBUG in checkHealth: Timecheck\n" << std::endl;
		if (it->second.CheckTime() == -1)
		{
			closeClient(it->first);
			it = _clientList.erase(it);
			printf("Connection timed out after 15 seconds of inactivity\n");
		}
		else
			it++;
	}
}

void Server::printBuffers()
{
	std::cout << "----------All stored data-----------" << std::endl;
	for (const auto& pair : _clientList)
	{
		std::cout << "FD = " << pair.first << std::endl;
		std::cout << "Buffer =\n" << pair.second._buf << "\n" << std::endl;
	}
}

// 
void Server::parse(int clientFd)
{
	HttpParser			parser;
	ConnectionContext	ctx;
	// parser.appendData(ctx, data.c_str(), data.size());
	ctx.buffer = _clientList.at(clientFd)._buf;

	try
	{
		ParseStatus status = parser.parseRequest(ctx);

		if (status == ParseStatus::COMPLETE)
		{
			std::cout << "✅ Request parsed successfully!\n";
			std::cout << "Method: " << ctx.request.method << "\n";
			std::cout << "URI: " << ctx.request.uri << "\n";
			std::cout << "Version: " << ctx.request.version << "\n";
			std::cout << "Headers:\n";
			for (std::map<std::string, std::string>::const_iterator it = ctx.request.headers.begin();
				 it != ctx.request.headers.end(); ++it)
				std::cout << "  " << it->first << ": " << it->second << "\n";

			if (ctx.request.body.empty())
				std::cout << "(no body)\n";
			else
				std::cout << "Body: " << ctx.request.body << "\n";
		}
		else if (status == ParseStatus::INCOMPLETE)
			std::cout << "⚠️ Parsing incomplete – more data needed.\n";
		else
			std::cout << "❌ Parsing failed.\n";
	}
	catch (const std::exception &e)
	{
		std::cerr << "Exception during parsing: " << e.what() << "\n";
	}
}

// Getters & Setters

const std::vector<ServerConfig>&	Server::getServerConfigs() const
{
	return (this->_serverConfigs);
}

void	Server::setServerConfigs(std::vector<ServerConfig> serverConfigs)
{
	this->_serverConfigs = serverConfigs;
}
