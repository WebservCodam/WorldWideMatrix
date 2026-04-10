/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/15 15:04:10 by vknape        #+#    #+#                 */
/*   Updated: 2026/04/10 15:52:52 by lprieri       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"
#include "../parser/HttpParser.hpp"
#include "configparser/Configuration.hpp"
#pragma once

class Client;

class Server
{
	private:
		std::vector<int>			_serverFds;
		const int					_epfd;
		std::map<int, Client>		_clientList;
		std::vector<ServerConfig>	_serverConfigs;

	public:
		// Orthodox Canonical Form (missing copy constructor, copy assignment operator, and move...)
		Server(int epfd);
		~Server();

		// Getters & Setters
		// ServerConfig				getServerConfig(int server);
		// ServerConfig				getServerConfig(const std::string& serverName);
		std::vector<ServerConfig>	getServerConfigs();
		
		
		
		void	parse(int clientFd);
		void	initServer();
		void	registerServerFd(int serverFd);
		void	startServer();
		void	close_client(int fd);
		void	check_health();
		void	add_fd_map(int clientFd);
		void	connect_new(int serverFd);
		void	connect_in(int clientFd);
		void	connect_out(int clientFd);
		void	print_buffers();
};
