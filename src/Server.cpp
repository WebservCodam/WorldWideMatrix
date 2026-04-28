/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/01 10:59:15 by vknape        #+#    #+#                 */
/*   Updated: 2026/04/28 11:35:27 by lprieri       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"

Server::Server(const ServerConfig& serverConfig): _serverConfig(serverConfig) {}

void	Server::addListenFd(int listenFd)
{
	_listenFds.push_back(listenFd);
}

// void Server::closeClient(int fd)
// {
// 	epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL);
// 	close(fd);
// 	// _clientList.erase(fd);
// }
// void Server::addFdToClientList(int clientFd)
// {
// 	_clientList.emplace(clientFd, clientFd);
// }



// void	Server::printBuffers()
// {
// 	std::cout << "----------All stored data-----------" << std::endl;
// 	for (const auto& pair : _clientList)
// 	{
// 		std::cout << "FD = " << pair.first << std::endl;
// 		std::cout << "Buffer =\n" << pair.second._buf << "\n" << std::endl;
// 	}
// }

// 
ParseStatus	Server::parse(int clientFd)
{
	HttpParser	parser;

	return (parser.initParser(_clientList.at(clientFd)));
}
