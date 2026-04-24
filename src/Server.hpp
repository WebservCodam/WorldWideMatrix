/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/15 15:04:10 by vknape        #+#    #+#                 */
/*   Updated: 2026/04/20 14:31:37 by lprieri       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "utils.hpp"
#include "../parser/HttpParser.hpp"
#include "configparser/Configuration.hpp"

# define EPOLL_NBR_EVENTS 1000

class Client;

class Server
{
	private:
		std::vector<int>			_listenFds;
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
		const std::vector<ServerConfig>&	getServerConfigs() const;
		void								setServerConfigs(std::vector<ServerConfig> serverConfigs);
		
		ParseStatus	parse(int clientFd);
		void		initServer();
		void		addListeningSocketToEpoll(int listenFd);
		void		startServer();
		void		closeClient(int fd);
		void		checkHealth();
		void		addFdToClientList(int clientFd);
		void		connectNew(int listenFd);
		void		connectIn(int clientFd);
		void		connectOut(int clientFd);
		void		printBuffers();
};
