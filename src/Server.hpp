/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/15 15:04:10 by vknape        #+#    #+#                 */
/*   Updated: 2026/04/29 18:44:40 by lprieri       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "utils.hpp"
#include "../parser/HttpParser.hpp"
#include "configparser/Configuration.hpp"

class Client;

class Server
{
	private:
		std::vector<int>		_listenFds;
		const ServerConfig&		_serverConfig;

	public:
		// Orthodox Canonical Form (missing copy constructor, copy assignment operator, and move...)
		Server() = delete;
		Server(const ServerConfig& serverConfig);
		~Server() = default;

		// Getters & Setters
		void	addListenFd(int listenFd);

		const ServerConfig&		getServerConfig() const { return _serverConfig; };
		const std::vector<int>&	getListenFds() const { return _listenFds; };

		void	handleRequest(Client& client);
};
