/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/15 14:47:31 by vknape        #+#    #+#                 */
/*   Updated: 2026/04/11 14:52:52 by lprieri       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "utils.hpp"
#include "Server.hpp"

#include "configparser/Configuration.hpp"

// void	startServer(int serverFd, int epfd);

void	printErrorAndExit(const std::string& msg, int errorCode)
{
	std::cerr << msg << std::endl;
	exit(errorCode);
}

void	initialize(int argc, char **argv, std::vector<ServerConfig>& configurations)
{
	if (argc != 2)
		printErrorAndExit("Error: Expecting an input file.", EXIT_FAILURE);

	std::ifstream	file(argv[1]);
	if (!file)
		printErrorAndExit("Error: Could not open file.", EXIT_FAILURE);

	std::stringstream	buffer;
	buffer << file.rdbuf();
	std::string input = buffer.str();
	
	try
	{
		std::unique_ptr<ConfigFile> ast = Parser(input).parse();
		configurations = ast->createServers();
	}
	catch (const ConfigError& e)
	{
		printErrorAndExit(e.what(), EXIT_FAILURE);
	}
	catch (const std::exception& e)
	{
		printErrorAndExit(std::string("Unexpected error\n") + e.what(), EXIT_FAILURE);
	}
}

int main(int argc, char** argv)
{
	std::vector<ServerConfig>	configurations;

	initialize(argc, argv, configurations);
	while (true)
	{
		try
		{
			int epfd;

			epfd = epoll_create(1000);
			
			if (epfd < 0)
				throw std::runtime_error("Failed to create epoll fd");
				
			Server	server(epfd);
			
			server.getServerConfigs() = configurations;
			std::cout << configurations.at(0).getServerName() << std::endl;
			
			server.initServer();
			server.startServer();
			
		}	catch (const std::runtime_error& e) {
				std::cout << "Runtime error: " << e.what() << std::endl;
		}	catch (const std::exception& e) {
				std::cout << "Exception: " << e.what() << std::endl;
		}
		
		exit(0);
	}
}

// void	startServer(int serverFd, int epfd)
// {
// 	Server server(serverFd, epfd);
// 	epoll_event events[1000];
// 	int num_events = 0;
// 	int connections = 0;
// 	while (true)
// 	{
// 		printf("Connections made = %d\n", connections);
// 		server.printBuffers();
// 		num_events = epoll_wait(epfd, events, 1000, 5000);
// 		printf("Number of events waiting: %d\n", num_events);
// 		if (num_events == -1)
// 			throw std::runtime_error("Epoll_wait failed");
		
// 		for (int i = 0; i < num_events; i++)
// 		{
// 			if (events[i].data.fd == serverFd)
// 			{
// 				server.connectNew();
// 				connections++;
// 			}

// 			else if (events[i].events & EPOLLIN)
// 			{
// 				server.connectIn(events[i].data.fd);
// 			}
			
// 			else if (events[i].events & EPOLLOUT)
// 			{
// 				server.connectOut(events[i].data.fd);
// 			}
// 		}
// 		server.checkHealth();
// 	}
// }