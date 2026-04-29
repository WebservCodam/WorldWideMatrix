/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/15 15:04:10 by vknape        #+#    #+#                 */
/*   Updated: 2026/04/29 13:21:29 by lprieri       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"
#include "utils.hpp"
#include "../parser/HttpParser.hpp"
#include "configparser/Configuration.hpp"



class Client;

class Server
{
	private:
		std::vector<int>		_listenFds;
		const ServerConfig&		_serverConfig;
		// std::map<int, Client>	_clients;

	public:
		// Orthodox Canonical Form (missing copy constructor, copy assignment operator, and move...)
		Server() = delete;
		Server(const ServerConfig& serverConfig);
		~Server() = default;

		// Getters & Setters
		void	addListenFd(int listenFd);
		// void	addClientFd(int clientFd);
		// void	removeClientFd(int clientFd);

		const ServerConfig&		getServerConfig() const { return _serverConfig; };
		// const Client&			getClient(int fd) const;
		const std::vector<int>&	getListenFds() const { return _listenFds; };
		
		
		// ParseStatus	parse(int clientFd);
		// void		initServer();
		// void		addListeningSocketToEpoll(int listenFd);
		// void		startServer();
		void		closeClient(int fd);
		// void		checkHealth();
		void		addClientFd(int clientFd, int listenFd);
		// void		connectNew(int listenFd);
		// void		connectIn(int clientFd);
		// void		connectOut(int clientFd);
		// void		printBuffers();
};
