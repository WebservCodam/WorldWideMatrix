/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/15 14:47:31 by vknape        #+#    #+#                 */
/*   Updated: 2026/02/17 14:44:57 by lprieri       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "utils.hpp"
#include "Server.hpp"

#include "configparser/include/Configuration.hpp"

// void	start_server(int server_fd, int epfd);

void	initialize(int argc, char **argv, std::unique_ptr<ConfigFile>& ast)
{
	if (argc != 2)
	{
		std::cerr << "Error: Expecting an input file." << std::endl;
		exit(EXIT_FAILURE);
	}

	std::ifstream	file(argv[1]);
	if (!file)
	{
		std::cerr << "Error: Could not open file" << std::endl;
		exit(EXIT_FAILURE);
	}

	std::stringstream	buffer;
	buffer << file.rdbuf();
	std::string input = buffer.str();
	
	try
	{
		ast = Parser(input).parse();
		if (!ast)
		{
			std::cerr << "Error: Failed to parse configuration" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	catch (const ConfigError& e)
	{
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Unexpected error: " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}

	// Phase 4: Create servers
	ast->createServers();

	std::cout << "Servers created" << std::endl;
}

int main(int argc, char** argv)
{
	std::unique_ptr<ConfigFile>	ast = NULL;

	initialize(argc, argv, ast);
	while (true)
	{
		try {
			int epfd;

			epfd = epoll_create(1000);
			
			if (epfd < 0)
				throw std::runtime_error("Failed to create epoll fd");
				
			Server server(epfd);
			server.servers = ast->getServers();
			std::cout << server.servers.at(0).getServerName() << std::endl;
			
			server.init_server();
			server.start_server();
			
		}	catch (const std::runtime_error& e) {
			std::cout << "Runtime error: " << e.what() << std::endl;
		}	catch (const std::exception& e) {
			std::cout << "Exception: " << e.what() << std::endl;
		}
		
		exit(0);
	}
}

// void	start_server(int server_fd, int epfd)
// {
// 	Server server(server_fd, epfd);
// 	epoll_event events[1000];
// 	int num_events = 0;
// 	int connections = 0;
// 	while (true)
// 	{
// 		printf("Connections made = %d\n", connections);
// 		server.print_buffers();
// 		num_events = epoll_wait(epfd, events, 1000, 5000);
// 		printf("Number of events waiting: %d\n", num_events);
// 		if (num_events == -1)
// 			throw std::runtime_error("Epoll_wait failed");
		
// 		for (int i = 0; i < num_events; i++)
// 		{
// 			if (events[i].data.fd == server_fd)
// 			{
// 				server.connect_new();
// 				connections++;
// 			}

// 			else if (events[i].events & EPOLLIN)
// 			{
// 				server.connect_in(events[i].data.fd);
// 			}
			
// 			else if (events[i].events & EPOLLOUT)
// 			{
// 				server.connect_out(events[i].data.fd);
// 			}
// 		}
// 		server.check_health();
// 	}
// }