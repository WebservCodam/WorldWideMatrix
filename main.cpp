/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vknape <vknape@student.codam.nl>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 14:47:31 by vknape            #+#    #+#             */
/*   Updated: 2025/09/18 13:58:18 by vknape           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "utils.hpp"
#include "Server.hpp"

int main()
{
	int server_fd, epfd;
	if (init_server(server_fd, epfd) == -1)
		perror("Server init failed");

	Server server(server_fd, epfd);
	epoll_event events[1000];
	int num_events = 0;
	int connections = 0;
	while (true)
	{
		printf("Connections made = %d\n", connections);
		print_buffers(server);
		num_events = epoll_wait(epfd, events, 1000, 5000);
		printf("Number of events waiting: %d\n", num_events);
		if (num_events == -1)
			exit(1);
		
		for (int i = 0; i < num_events; i++)
		{
			if (events[i].data.fd == server_fd)
			{
				if (connect_new(server))
					;
				connections++;
			}

			else if (events[i].events & EPOLLIN)
			{
				if (connect_in(events[i].data.fd, server))
					;
			}
			
			else if (events[i].events & EPOLLOUT)
			{
				if (connect_out(events[i].data.fd, server))
					;
			}
		}
		check_health(server);
		
	}
	close(server_fd);
}