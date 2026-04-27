/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/15 15:04:10 by vknape        #+#    #+#                 */
/*   Updated: 2026/04/27 13:50:28 by lprieri       ########   odam.nl         */
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
		std::vector<int>		_listenFds;
		std::map<int, Client>	_clientList;
		const ServerConfig&		_serverConfig;

	public:
		// Orthodox Canonical Form (missing copy constructor, copy assignment operator, and move...)
		Server() = delete;
		Server(const ServerConfig& serverConfig);
		~Server() = default;

		// Getters & Setters
		void					addListenFd(int listenFd);

		const ServerConfig&		getServerConfig() const;
		const Client&			getClient(int fd) const;
		const std::vector<int>&	getListenFds() const;
		
		
		// ParseStatus	parse(int clientFd);
		// void		initServer();
		// void		addListeningSocketToEpoll(int listenFd);
		// void		startServer();
		void		closeClient(int fd);
		// void		checkHealth();
		void		addFdToClientList(int clientFd);
		// void		connectNew(int listenFd);
		// void		connectIn(int clientFd);
		// void		connectOut(int clientFd);
		// void		printBuffers();
};
