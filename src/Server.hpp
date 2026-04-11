/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/15 15:04:10 by vknape        #+#    #+#                 */
/*   Updated: 2026/04/11 14:52:52 by lprieri       ########   odam.nl         */
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
		void	closeClient(int fd);
		void	checkHealth();
		void	addFdToClientList(int clientFd);
		void	connectNew(int serverFd);
		void	connectIn(int clientFd);
		void	connectOut(int clientFd);
		void	printBuffers();
};
