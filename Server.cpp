/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vknape <vknape@student.codam.nl>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/01 10:59:15 by vknape            #+#    #+#             */
/*   Updated: 2025/09/18 14:10:59 by vknape           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"

Server::Server(int server_fd1, int epfd1) : server_fd(server_fd1), epfd(epfd1) {};

Server::~Server() {};

int connect_new(Server& server)
{
	int client_fd;
	while (true)
	{
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		client_fd = accept(server.server_fd, (struct sockaddr*)&client_addr, &client_len);
		
		if (client_fd == -1)
		{
			perror("Accept failed");
			return (-1);
		}
		
		if (set_non_blocking(client_fd) == -1)
		{
			perror("Set flags failed");
			return (-1);
		}
		if (add_fd_map(client_fd, server) == -1)
		{
			perror("Failed to create and add Client object");
			return (-1);
		}
		struct epoll_event event;
		event.events = EPOLLIN | EPOLLET;
		event.data.fd = client_fd;
		if (epoll_ctl(server.epfd, EPOLL_CTL_ADD, client_fd, &event) == -1)
		{
			perror("Epoll_ctl: add client_fd failed");
			close(client_fd);
		}
	}
}

int connect_in(int client_fd, Server& server)
{
	printf("Read start\n");
	char buffer[8192] = {0};
	ssize_t count = 0;

	count = recv(client_fd, buffer, sizeof(buffer), 0);
	server.list.at(client_fd)._buf += buffer;
	// write(STDOUT_FILENO, buffer, count);
		
	// perror("Read error: ");
	if (count == 0)
	{
		printf("Client %d disconnected\n", client_fd);
		close(client_fd);
		server.list.erase(client_fd);
		return (0);
	}
	else if (count == -1)
	{
		// perror("Read failed");
		// close(client_fd);
		server.list.at(client_fd).readstate = count;
	}
	struct epoll_event event;
	event.events = EPOLLOUT | EPOLLET;
	event.data.fd = client_fd;
	if (epoll_ctl(server.epfd, EPOLL_CTL_MOD, client_fd, &event) == -1)
	{
		perror("Epoll_ctl: switch to EPOLLOUT failed");
		close(client_fd);
	}
	printf("Read still open\n");
	return (-1);
}

int connect_out(int client_fd, Server& server)
{
	printf("Writing now\n");
	std::ifstream file_stream("index.html");
	// if (!file_stream.is_open()) {
	//     return ""; // Return empty string if the file can't be opened
	// }
	std::stringstream buffer;
	buffer << file_stream.rdbuf();
	std::string html =  buffer.str();	
	
	
	// const char* hello = "Hello from the server";
	// const char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
	std::string header = "HTTP/1.1 200 OK\nContent-Type: text/html\n";
	header += "Content-Length: " + std::to_string(html.length()) + "\n\n";
	std::string response = header + html;
	
	// write(client_fd, "hello", 5);
	// // close(client_fd);
	write(client_fd, response.c_str(), response.length());
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = client_fd;
	if (epoll_ctl(server.epfd, EPOLL_CTL_MOD, client_fd, &event) == -1)
	{
		perror("Epoll_ctl: switch to EPOLLIN failed");
		close(client_fd);
	}
	return (0);
}

void check_health(Server& server)
{
	for (auto it = server.list.begin(); it != server.list.end();)
	{
		printf("check1\n");
		if (it->second.CheckTime() == -1)
		{
			printf("here\n");
			epoll_ctl(server.epfd, EPOLL_CTL_DEL, it->first, NULL);
			close(it->first);
			it = server.list.erase(it);
		}
		else
			it++;
		printf("check2\n");
	}
}

void print_buffers(Server& server)
{
	std::cout << "----------All stored data-----------" << std::endl;
	for (const auto& pair : server.list)
	{
		std::cout << "FD = " << pair.first << std::endl;
		std::cout << "Buffer =\n" << pair.second._buf << "\n" << std::endl;
	}
}