/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vknape <vknape@student.codam.nl>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 14:20:54 by vknape            #+#    #+#             */
/*   Updated: 2025/09/18 13:31:54 by vknape           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

int	init_server(int& server_fd, int& epfd)
{
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Cannot create socket");
		return (0);
	}
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&reuse, sizeof(int)) == -1)
	{
		perror("setsockopt");
		/* Handle error here */
	}
	struct sockaddr_in address;
	memset((char*)&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(8080);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
	{
		perror("Bind failed");
		return (0);
	}
	
	set_non_blocking(server_fd);
	if (listen(server_fd, 1000) < 0)
	{
		perror("In listen");
		exit(EXIT_FAILURE);
	}
		
	//epoll start
	epfd = epoll_create(1000);
	static struct epoll_event event;
	event.data.fd = server_fd;
	event.events = EPOLLIN | EPOLLET | EPOLLPRI | EPOLLHUP;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &event) == -1)
		return (-1);
	return (0);
}