/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/01 10:59:15 by vknape        #+#    #+#                 */
/*   Updated: 2026/04/27 13:58:05 by lprieri       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"

Server::Server(const ServerConfig& serverConfig): _serverConfig(serverConfig) {}

void	Server::addListenFd(int listenFd)
{
	_listenFds.push_back(listenFd);
}

const std::vector<int>&	Server::getListenFds() const
{
	return (_listenFds);
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

	std::cout << "DEBUG in connnectNew" << std::endl;
	getServerConfig(listenFd);

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
	char		buffer[8192] = {0};
	ssize_t		count = 0; // Bytes read.
	ParseStatus	status;

	count = recv(clientFd, buffer, sizeof(buffer), 0);

	if (count == 0 || count == -1)
	{
		if (count == -1)
			perror("recv failed");
		close(clientFd); // Closing removes from epoll, so no epoll_ctl is needed to remove it.
		_clientList.erase(clientFd);
		return ;
	}

	_clientList.at(clientFd).setTime(); // Reset time after a succesful read.
	_clientList.at(clientFd)._buf.append(buffer, count); // Changed this line to the CPP version.
	// write(STDOUT_FILENO, buffer, count);
	
	std::cout << "DEBUG in connectIn: PRINTING BUFFER" << std::endl;
	std::cout << buffer << std::endl;

	status = parse(clientFd); // Branch the status received into conditionals.

	if (status == INCOMPLETE)
	{
		return ; // So it goes back to connectIn on next loop. And since it's level-triggered, the event will still be there.
	}
	else if (status == ERROR)
	{
		std::cerr << "DEBUG: ERROR thrown while parsing HTTP request." << std::endl;
		// And we want the event to be switched to EPOLLOUT?
	}

	//else -> status == COMPLETE and we can switch the epoll event to EPOLLOUT

	std::cout << "DEBUG in connectIn" << std::endl;
	std::cout << "\n ---------- PARSE END ----------\n" << std::endl;

	struct epoll_event	event;

	//Before flipping to EPOLLOUT, is the parsing complete and without errors?
	event.events = EPOLLOUT;
	event.data.fd = clientFd;
	if (epoll_ctl(_epfd, EPOLL_CTL_MOD, clientFd, &event) == -1) // When this function goes out of scope, what happens to event? It gets transfered? Does epoll_ctl copy it?
	{
		perror("Epoll_ctl: switch to EPOLLOUT failed");
		close(clientFd);
	}


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

	while (it != _clientList.end()) // Since the iterator was incremented within the loop, I changed this to a while loop.
	{
		std::cout << "DEBUG in checkHealth: Timecheck\n" << std::endl;
		if (it->second.checkTime() == -1)
		{
			closeClient(it->first);
			it = _clientList.erase(it);
			printf("Connection timed out after 15 seconds of inactivity\n");
		}
		else
			it++;
	}
}

void	Server::printBuffers()
{
	std::cout << "----------All stored data-----------" << std::endl;
	for (const auto& pair : _clientList)
	{
		std::cout << "FD = " << pair.first << std::endl;
		std::cout << "Buffer =\n" << pair.second._buf << "\n" << std::endl;
	}
}

// 
ParseStatus	Server::parse(int clientFd)
{
	HttpParser	parser;

	return (parser.initParser(_clientList.at(clientFd)));
}

// Getters & Setters

const	std::vector<ServerConfig>&	Server::getServerConfigs() const
{
	return (this->_serverConfigs);
}

const ServerConfig&	Server::getServerConfig(int listenFd) const
{
	struct sockaddr_in	addr;
	socklen_t			sockLen = sizeof(addr);
	uint16_t			port;
	uint16_t			address;

	if (getsockname(listenFd, (struct sockaddr*) &addr, &sockLen) != 0)
		throw std::runtime_error("Error retrieving the socket address in getServerConfig");

	port = ntohs(addr.sin_port);	// Network-byte order (Big endian) to Host byte order (depends on operating system).
	address = ntohs(addr.sin_addr.s_addr);
	std::cout << "Port: " << std::to_string(port) << std::endl;

	return (this->getServerConfigs().at(0)); // Temp
}

void	Server::setServerConfigs(std::vector<ServerConfig> serverConfigs)
{
	this->_serverConfigs = serverConfigs;
}
