#pragma once

#include "utils.hpp"
#include "httpparser/HttpParser.hpp"
#include "configparser/Configuration.hpp"
#include "configparser/ServerConfig.hpp"
#include "Server.hpp"
#include "Client.hpp"

# define EPOLL_NBR_EVENTS 1000
# define EPOLL_TIMEOUT 5000

class Webserv
{
	private:
		const int					_epfd;
		std::vector<ServerConfig>	_serverConfigs;
		std::vector<Server>			_servers;
		std::map<int, Client>		_clients;
		std::map<int, Server*>		_listenFdToServer;
		std::map<int, Server*>		_clientFdToServer; // Webserv owns the clients map, to avoid cross-class fd ownership puzzles.

	public:
		Webserv() = delete;
		Webserv(int epfd): _epfd(epfd) {};
		~Webserv() { close(_epfd); };

		const std::vector<ServerConfig>&	getServerConfigs() const { return _serverConfigs; };
		void								setServerConfigs(std::vector<ServerConfig> serverConfigs) { this->_serverConfigs = serverConfigs; };

		void	addFdToClientList(int clientFd, int listenFd);
		void	closeAndRemoveFdFromClientList(int clientFd);

		void	initWebserv();
		void	addListeningSocketToEpoll(int listenFd);
		void	startServers();
		void	connectNew(int listenFd);
		void	connectIn(int clientFd);
		void	connectOut(int clientFd);
		void	checkHealth();

		ParseStatus	parse(int clientFd);
};