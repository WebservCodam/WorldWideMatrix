/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/01 10:59:15 by vknape        #+#    #+#                 */
/*   Updated: 2026/05/19 17:59:37 by lprieri       ########   odam.nl         */
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

	try
	{
		if (client._request.method == "GET")
		{
			std::cout << "DEBUG: URI: " << client._request.uri << std::endl;
			// std::vector<Location>	locations = ;

			// client._response.headers = "HTTP/1.1 200 OK\nContent-Type: text/html\n";
			client._response.body = "hello world\n";

			const Location& location = _serverConfig.getLocation(client._request.uri);

			std::ifstream		file_stream(location.indexPath);
			std::stringstream	buffer;
		
		// if (!file_stream.is_open()) {
		//     return ""; // Return empty string if the file can't be opened
		// }
		
			client._response.body = buffer.str();
			file_stream.close();
			client._response.headers.insert(std::make_pair("header", "HTTP/1.1 200 OK\nContent-Type: text/html\n Content-Length: " + std::to_string(client._request.body.length()) + "\n\n"));
		}
	}
	catch (std::runtime_error& e)
	{
		std::cout << "Caught exception: " << e.what() << std::endl;
	}
 
	// std::cout << "Version: " << client._request.version << std::endl;
	// std::cout << "Method: " << client._request.method << std::endl;
	// std::cout << "URI: " << client._request.uri << std::endl;
	// for (const auto& header : client._request.headers)
	// {
	// 	std::cout << "Header: " << header.first << "=>" << header.second << std::endl;
	// }
	// std::cout << "Body: " << client._request.body << std::endl;
}

