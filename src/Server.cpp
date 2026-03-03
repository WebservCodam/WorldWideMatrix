/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/01 10:59:15 by vknape        #+#    #+#                 */
/*   Updated: 2026/03/03 18:51:13 by lprieri       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"

Server::Server(int epfd1) : epfd(epfd1) {};

Server::~Server() {close(epfd);};

void Server::init_server()
{
	for (auto server: servers)
	{
		std::vector<ListenDirective> addresses = server.getListenDirectives();
		auto it = addresses.begin();
		server_fds.push_back(createSocket(it->address.c_str(), it->port.c_str()));
		// break ;
	}

	for (auto fd: server_fds)
	{
		add_servers_to_epoll(fd);
	}
}

void Server::add_servers_to_epoll(int server_fd)
{
		//epoll start
	static struct epoll_event event;
	event.data.fd = server_fd;
	event.events = EPOLLIN | EPOLLET | EPOLLPRI | EPOLLHUP;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &event) == -1)
		throw std::runtime_error("Server add to epoll failed");
}

void Server::start_server()
{
	Server		server(epfd);
	epoll_event	events[1000];
	int			num_events = 0;
	int			connections = 0;

	while (true)
	{
		// printf("Connections made = %d\n", connections);
		// server.print_buffers();
		num_events = epoll_wait(epfd, events, 1000, 5000);
		// printf("Number of events waiting: %d\n", num_events);
		if (num_events == -1)
			throw std::runtime_error("Epoll_wait failed");
		
		for (int i = 0; i < num_events; i++)
		{
			if (std::find(server_fds.begin(), server_fds.end(), events[i].data.fd) != server_fds.end())
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
	epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
	// list.erase(fd);
}
void Server::add_fd_map(int client_fd)
{
	list.emplace(client_fd, client_fd);	// Client fd as second argument?
}

void Server::connect_new(int server_fd)
{
	int client_fd;
	while (true)
	{
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
		
		if (client_fd == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return ;
			else
				throw std::runtime_error("Client accept failed");
		}
		
		if (set_non_blocking(client_fd) == -1)
		{
			// perror("Set flags failed");
			// return (-1);
			throw std::runtime_error("Client set flags failed");
		}
		try {
			add_fd_map(client_fd);
		} catch (const std::exception& e) {
			std::cout << "Failed to create and add Client object: " << e.what() << std::endl;
			throw;
		}
		struct epoll_event event;
		event.events = EPOLLIN | EPOLLET;
		event.data.fd = client_fd;
		if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &event) == -1)
		{
			// perror("Epoll_ctl: add client_fd failed");
			// close(client_fd);
			
			throw std::runtime_error("Failed to add client to epoll");
		}
		// printf("here\n");
	}
}

void Server::connect_in(int client_fd)
{
	printf("\n----------------------------------------\nRead start\n");
	char buffer[8192] = {0};
	ssize_t count = 0;

	count = recv(client_fd, buffer, sizeof(buffer), 0);
	list.at(client_fd)._buf += buffer;
	// write(STDOUT_FILENO, buffer, count);
	printf("%s\n", buffer);
		
	// perror("Read error: ");
	// if (count == 0)
	// {
	// 	printf("Client %d disconnected\n", client_fd);
	// 	close(client_fd);
	// 	server.list.erase(client_fd);
	// 	return (0);
	// }
	// else if (count == -1)
	// {
	// 	// perror("Read failed");
	// 	// close(client_fd);
	// 	server.list.at(client_fd).readstate = count;
	// }
	parse(client_fd);
	printf("\n----------------------------------------\nParse end\n\n");
	struct epoll_event event;
	event.events = EPOLLOUT | EPOLLET;
	event.data.fd = client_fd;
	if (epoll_ctl(epfd, EPOLL_CTL_MOD, client_fd, &event) == -1)
	{
		perror("Epoll_ctl: switch to EPOLLOUT failed");
		close(client_fd);
	}
	// printf("Read still open\n");
}

void Server::connect_out(int client_fd)
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
	
	// write(client_fd, "hello", 5);
	// // close(client_fd);
	list.at(client_fd)._buf.clear();
	write(client_fd, response.c_str(), response.length());
	if (list.at(client_fd)._alive == false)
	{
		close_client(client_fd);
		list.erase(client_fd);
		return;
	}
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = client_fd;
	if (epoll_ctl(epfd, EPOLL_CTL_MOD, client_fd, &event) == -1)
	{
		perror("Epoll_ctl: switch to EPOLLIN failed");
		close(client_fd);
	}
}

void Server::check_health()
{
	for (auto it = list.begin(); it != list.end();)
	{
		printf("Timecheck\n");
		if (it->second.CheckTime() == -1)
		{
			close_client(it->first);
			it = list.erase(it);
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
	for (const auto& pair : list)
	{
		std::cout << "FD = " << pair.first << std::endl;
		std::cout << "Buffer =\n" << pair.second._buf << "\n" << std::endl;
	}
}

// 
void Server::parse(int client_fd)
{
	HttpParser parser;
	ConnectionContext ctx;
	// parser.appendData(ctx, data.c_str(), data.size());
	ctx.buffer = list.at(client_fd)._buf;

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