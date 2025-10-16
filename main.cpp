/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vknape <vknape@student.codam.nl>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 14:47:31 by vknape            #+#    #+#             */
/*   Updated: 2025/10/16 11:12:01 by vknape           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "utils.hpp"
#include "Server.hpp"

void	start_server(int server_fd, int epfd);

int main()
{
	int server_fd, epfd;
	
	while (true)
	{
		try {
			init_server(server_fd, epfd);
			start_server(server_fd, epfd);
		}	catch (const std::runtime_error& e) {
			std::cout << "Runtime error: " << e.what() << std::endl;
		}	catch (const std::exception& e) {
			std::cout << "Exception: " << e.what() << std::endl;
		}
	}
	
}

void	start_server(int server_fd, int epfd)
{
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
			throw std::runtime_error("Epoll_wait failed");
		
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
}