/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/01 10:59:15 by vknape        #+#    #+#                 */
/*   Updated: 2026/04/10 15:53:01 by lprieri       ########   odam.nl         */
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
	for (ServerConfig server : _serverConfigs)
	{
		std::vector<ListenDirective>	addresses = server.getListenDirectives();

		auto it = addresses.begin();
		_serverFds.push_back(createSocket(it->address.c_str(), it->port.c_str()));
		// break ;
	}

	for (int fd: _serverFds)
	{
		registerServerFd(fd);
	}
}

/**
 * Adds the server fd to epoll.
 */
void Server::registerServerFd(int serverFd)
{
	static struct epoll_event	event;

	event.data.fd = serverFd;
	event.events = EPOLLIN | EPOLLET | EPOLLPRI | EPOLLHUP;
	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, serverFd, &event) == -1)
		throw std::runtime_error("Server add to epoll failed");
}

void Server::startServer()
{
	Server		server(_epfd);
	epoll_event	events[1000];
	int			numEvents = 0;
	int			connections = 0;

	while (true)
	{
		// printf("Connections made = %d\n", connections);
		// server.print_buffers();
		numEvents = epoll_wait(_epfd, events, 1000, 5000);
		// printf("Number of events waiting: %d\n", numEvents);
		if (numEvents == -1)
			throw std::runtime_error("Epoll_wait failed");
		
		for (int i = 0; i < numEvents; i++)
		{
			if (std::find(_serverFds.begin(), _serverFds.end(), events[i].data.fd) != _serverFds.end())
			{
				server.connect_new(events[i].data.fd);
				connections++;
			}

			else if (events[i].events & EPOLLIN)
			{
				server.connect_in(events[i].data.fd);
			}
			
			else if (events[i].events & EPOLLOUT)
			{
				server.connect_out(events[i].data.fd);
			}
		}
		server.check_health();
	}
}

void Server::close_client(int fd)
{
	epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
	// _clientList.erase(fd);
}
void Server::add_fd_map(int clientFd)
{
	_clientList.emplace(clientFd, clientFd);
}

void Server::connect_new(int serverFd)
{
	int	clientFd;
	while (true)
	{
		struct sockaddr_in	clientAddr;
		socklen_t	clientLen = sizeof(clientAddr);

		clientFd = accept(serverFd, (struct sockaddr*)&clientAddr, &clientLen);
		
		if (clientFd == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return ;
			else
				throw std::runtime_error("Client accept failed");
		}
		
		if (set_non_blocking(clientFd) == -1)
		{
			// perror("Set flags failed");
			// return (-1);
			throw std::runtime_error("Client set flags failed");
		}
		try {
			add_fd_map(clientFd);
		} catch (const std::exception& e) {
			std::cout << "Failed to create and add Client object: " << e.what() << std::endl;
			throw;
		}
		struct epoll_event event;
		event.events = EPOLLIN | EPOLLET;
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

void Server::connect_in(int clientFd)
{
	printf("\n----------------------------------------\nRead start\n");
	char buffer[8192] = {0};
	ssize_t count = 0;

	count = recv(clientFd, buffer, sizeof(buffer), 0);
	_clientList.at(clientFd)._buf += buffer;
	// write(STDOUT_FILENO, buffer, count);
	printf("%s\n", buffer);
		
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
	printf("\n----------------------------------------\nParse end\n\n");
	struct epoll_event event;
	event.events = EPOLLOUT | EPOLLET;
	event.data.fd = clientFd;
	if (epoll_ctl(_epfd, EPOLL_CTL_MOD, clientFd, &event) == -1)
	{
		perror("Epoll_ctl: switch to EPOLLOUT failed");
		close(clientFd);
	}
	// printf("Read still open\n");
}

void Server::connect_out(int clientFd)
{
	printf("Writing now\n");
	std::ifstream file_stream("index.html");
	// if (!file_stream.is_open()) {
	//     return ""; // Return empty string if the file can't be opened
	// }
	std::stringstream buffer;
	buffer << file_stream.rdbuf();
	std::string html =  buffer.str();
	file_stream.close();
	
	
	// const char* hello = "Hello from the server";
	// const char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
	std::string header = "HTTP/1.1 200 OK\nContent-Type: text/html\n";
	header += "Content-Length: " + std::to_string(html.length()) + "\n\n";
	std::string response = header + html;
	
	// write(clientFd, "hello", 5);
	// // close(clientFd);
	_clientList.at(clientFd)._buf.clear();
	write(clientFd, response.c_str(), response.length());
	if (_clientList.at(clientFd)._alive == false)
	{
		close_client(clientFd);
		_clientList.erase(clientFd);
		return;
	}
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = clientFd;
	if (epoll_ctl(_epfd, EPOLL_CTL_MOD, clientFd, &event) == -1)
	{
		perror("Epoll_ctl: switch to EPOLLIN failed");
		close(clientFd);
	}
}

void Server::check_health()
{
	for (auto it = _clientList.begin(); it != _clientList.end();)
	{
		printf("Timecheck\n");
		if (it->second.CheckTime() == -1)
		{
			close_client(it->first);
			it = _clientList.erase(it);
			printf("Connection timed out after 15 seconds of inactivity\n");
		}
		else
			it++;
		// printf("check2\n");
	}
}

void Server::print_buffers()
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
	HttpParser parser;
	ConnectionContext ctx;
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

std::vector<ServerConfig>	Server::getServerConfigs()
{
	return (this->_serverConfigs);
}
