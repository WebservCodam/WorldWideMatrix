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
		
		void	servePost(HttpResponse& res, const std::string& body, const Location& location, const std::string& remainder);
		void	serveDelete(HttpResponse& res, const std::string& fsPath);


	public:
		Server() = delete;
		Server(const ServerConfig& serverConfig);
		~Server() = default;

		// Getters & Setters
		void	addListenFd(int listenFd);

		const ServerConfig&		getServerConfig() const { return _serverConfig; };
		const std::vector<int>&	getListenFds() const { return _listenFds; };

		bool  		isCgiRequest(const std::string& uri);
		void 		handleRequest(Client& client);
		std::string	resolveFsPath(const HttpRequest& request) const;
		std::string	resolveScriptPath(const std::string& uri) const;
		void		serveErrorPage(HttpResponse& res, int code);
		void		serveReturn(HttpResponse& res, const ReturnPage& ret);
};
