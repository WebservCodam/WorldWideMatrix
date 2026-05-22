/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/01 10:59:15 by vknape        #+#    #+#                 */
/*   Updated: 2026/05/20 10:46:08 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"

Server::Server(const ServerConfig& serverConfig): _serverConfig(serverConfig) {}

void	Server::addListenFd(int listenFd)
{
	_listenFds.push_back(listenFd);
}

// Reads the entire file at `path` into `out`.
// Returns false if the file can't be opened, true on success.
static bool	loadFile(const std::string& path, std::string& out)
{
	std::ifstream	file(path);

	if (!file.is_open())
		return (false);

	std::stringstream	buffer;
	buffer << file.rdbuf();
	out = buffer.str();
	return (true);
}

// Fills `res` with an error page for `code`: tries the error page file
// configured for that code, and falls back to a built-in HTML string.
void	Server::serveErrorPage(HttpResponse& res, int code)
{
	res.status = code;
	try
	{
		ErrorPage	errorPage = _serverConfig.getErrorPage(code);
		if (loadFile(errorPage.URI, res.body))
			return ;
	}
	catch (const std::exception&)
	{
	}
	res.body = "<html><body><h1>" + std::to_string(code)
		+ " Error</h1></body></html>";
}

void	Server::handleRequest(Client& client)
{
	HttpResponse&	res = client._response;

	try
	{
		if (client._request.method != "GET")
		{
			serveErrorPage(res, 405);
			return ;
		}

		const Location&	location = _serverConfig.getLocation(client._request.uri);

		if (loadFile(location.indexPath, res.body))
			res.status = 200;
		else
			serveErrorPage(res, 404);
	}
	catch (const std::exception&)
	{
		serveErrorPage(res, 404);
	}
}
