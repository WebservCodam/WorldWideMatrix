/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   init.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/15 14:20:54 by vknape        #+#    #+#                 */
/*   Updated: 2026/04/11 17:07:04 by lprieri       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

// void	initServer(int& listenFd, int& epfd)
// {
// 	int epfd;

// 	epfd = epoll_create(1000);
// 	if (epfd < 0)
// 		throw std::runtime_error("Failed to create epoll fd");

// 	while (true)
// 	{
// 		servecreateSocket(NULL, 8080);
// 		break ;
// 	}
// 	//epoll start
// 	static struct epoll_event event;
// 	event.data.fd = listenFd;
// 	event.events = EPOLLIN | EPOLLET | EPOLLPRI | EPOLLHUP;
// 	if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenFd, &event) == -1)
// 		throw std::runtime_error("Server add to epoll failed");
// }

int createSocket(const char* ip, const char* port)
{
	struct addrinfo hints, *res;
	
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;			// What domain or address family?: IPv4 
	hints.ai_socktype = SOCK_STREAM;	// What type of service? Stream, datagram or raw?: Stream
	hints.ai_protocol = 0;				// What kind of protocol: Default protocol, because family and socket type already tells us it's the TCP protocol.
	hints.ai_flags = AI_PASSIVE;		// 
	
	// std::string portstr = std::to_string(port);
	// const char* port_str = portstr.c_str();

	int status = getaddrinfo(ip, port, &hints, &res);
	status = -1;
	std::cout << std::to_string(status) << std::endl;
	if (status != 0)
	{
		std::cout << "Do we come here?" << std::endl;
		throw std::runtime_error("Server socket addrinfo failed");
		return (-1);
	}

	int listenFd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (listenFd < 0)
	{
		freeaddrinfo(res);
		throw std::runtime_error("Server socket creation failed");
	}
	
	int opt = 1;
	if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		close(listenFd);
		freeaddrinfo(res);
		throw std::runtime_error("Server socket setsockopt failed");
	}
	
	if (bind(listenFd, res->ai_addr, res->ai_addrlen) < 0)
	{
		close(listenFd);
		freeaddrinfo(res);
		throw std::runtime_error("Server socket bind failed");
	}
	
	setNonBlocking(listenFd);
	freeaddrinfo(res);

	if (listen(listenFd, 1000) < 0)
	{
		close(listenFd);
		throw std::runtime_error("Server socket listen failed");
	}
	std::cout << "Server with address: " << ip << " at port: " << port << " created" << std::endl;
	return (listenFd);
}
