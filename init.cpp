/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vknape <vknape@student.codam.nl>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 14:20:54 by vknape            #+#    #+#             */
/*   Updated: 2025/11/03 14:57:50 by vknape           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

// void	init_server(int& server_fd, int& epfd)
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
// 	event.data.fd = server_fd;
// 	event.events = EPOLLIN | EPOLLET | EPOLLPRI | EPOLLHUP;
// 	if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &event) == -1)
// 		throw std::runtime_error("Server add to epoll failed");
// }

int createSocket(const char* ip, int port)
{
	struct addrinfo hints, *res;
	
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	std::string portstr = std::to_string(port);
	const char* port_str = portstr.c_str();

	int status = getaddrinfo(ip, port_str, &hints, &res);
	if (status != 0)
		throw std::runtime_error("Server socket addrinfo failed");

	int server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (server_fd < 0)
	{
		freeaddrinfo(res);
		throw std::runtime_error("Server socket creation failed");
	}
	
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		close(server_fd);
		freeaddrinfo(res);
		throw std::runtime_error("Server socket setsockopt failed");
	}
	
	if (bind(server_fd, res->ai_addr, res->ai_addrlen) < 0)
	{
		close(server_fd);
		freeaddrinfo(res);
		throw std::runtime_error("Server socket bind failed");
	}
	
	set_non_blocking(server_fd);
	freeaddrinfo(res);

	if(listen(server_fd, 1000) < 0)
	{
		close(server_fd);
		throw std::runtime_error("Server socket listen failed");
	}

	return (server_fd);
}