/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vknape <vknape@student.codam.nl>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 14:20:54 by vknape            #+#    #+#             */
/*   Updated: 2025/10/16 11:10:24 by vknape           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

void	init_server(int& server_fd, int& epfd)
{
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw std::runtime_error("Server socket creation failed");
		
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&reuse, sizeof(int)) == -1)
		throw std::runtime_error("Server socket setting failed");
		
	struct sockaddr_in address;
	memset((char*)&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(8080);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
		throw std::runtime_error("Server bind failed");
	
	set_non_blocking(server_fd);
	if (listen(server_fd, 1000) < 0)
		throw std::runtime_error("Server listen failed");
		
	//epoll start
	epfd = epoll_create(1000);
	static struct epoll_event event;
	event.data.fd = server_fd;
	event.events = EPOLLIN | EPOLLET | EPOLLPRI | EPOLLHUP;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &event) == -1)
		throw std::runtime_error("Server add to epoll failed");
}