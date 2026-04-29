/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/01 10:59:15 by vknape        #+#    #+#                 */
/*   Updated: 2026/04/29 18:54:01 by lprieri       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"

Server::Server(const ServerConfig& serverConfig): _serverConfig(serverConfig) {}

void	Server::addListenFd(int listenFd)
{
	_listenFds.push_back(listenFd);
}

void	Server::handleRequest(Client& client)
{
	std::cout << "DEBUG - handleRequest" << std::endl;
 
	std::cout << "Version: " << client._request.version << std::endl;
	std::cout << "Method: " << client._request.method << std::endl;
	std::cout << "URI: " << client._request.uri << std::endl;
	for (const auto& header : client._request.headers)
	{
		std::cout << "Header: " << header.first << "=>" << header.second << std::endl;
	}
	std::cout << "Body: " << client._request.body << std::endl;
}

