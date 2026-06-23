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
		const int							_epfd;
		std::vector<ServerConfig>			_serverConfigs;
		std::vector<Server>					_servers;
		std::map<int, Client>				_clients;
		std::map<int, std::vector<Server*>>	_listenFdToServers;	// One listening socket can front several servers (virtual hosts).
		
	public:
		Webserv() = delete;
		Webserv(int epfd): _epfd(epfd) {};
		~Webserv() { close(_epfd); };

		const std::vector<ServerConfig>&	getServerConfigs() const { return _serverConfigs; };
		void								setServerConfigs(std::vector<ServerConfig> serverConfigs) { this->_serverConfigs = serverConfigs; };

		void	addFdToClientList(int clientFd, int listenFd);
		void	closeAndRemoveFdFromClientList(int clientFd);

		void		initWebserv();
		int			getOrCreateListenSocket(const ListenDirective& listenDir, std::map<std::string, int>& hostPortToFd);
		bool		epollCtl(int op, int fd, uint32_t events);
		void		startServers();
		void		connectNew(int listenFd);
		void		connectIn(int clientFd);
		void		connectOut(int clientFd);
		Server*		selectServer(int listenFd, const std::string& host);
		std::string	getRequestHost(const Client& client);
		void		checkHealth();

		ParseStatus	parse(int clientFd);
};