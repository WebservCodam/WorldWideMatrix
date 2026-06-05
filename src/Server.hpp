#pragma once

#include <cctype>
#include <sys/types.h>
#include <dirent.h>
#include "utils.hpp"
#include "httpparser/HttpParser.hpp"
#include "configparser/Configuration.hpp"
#include "configparser/ServerConfig.hpp"

class Client;
struct HttpResponse;

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
		void	serveErrorPage(HttpResponse& res, int code);
		void	serveReturn(HttpResponse& res, const ReturnPage& ret);
};
