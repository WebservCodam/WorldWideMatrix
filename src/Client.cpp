/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Client.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/12 14:12:28 by vknape        #+#    #+#                 */
/*   Updated: 2026/05/20 10:41:30 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"

Client::Client(int fd) : _clientFd(fd), _time(0), _alive(false)
{
	setTime();
}

Client::~Client() 
{
	close(_clientFd);
}

// Maps a status code to its reason phrase.
static std::string	reasonPhrase(int status)
{
	switch (status)
	{
		case 200: return ("OK");
		case 201: return ("Created");
		case 301: return ("Moved Permanently");
		case 302: return ("Found");
		case 303: return ("See Other");
		case 307: return ("Temporary Redirect");
		case 308: return ("Permanent Redirect");
		case 400: return ("Bad Request");
		case 403: return ("Forbidden");
		case 404: return ("Not Found");
		case 405: return ("Method Not Allowed");
		case 413: return ("Payload Too Large");
		case 500: return ("Internal Server Error");
		default:  return ("Unknown");
	}
}

// Builds the complete HTTP response text: status line, headers,
// a blank line, then the body.
std::string	Client::serializeResponse()
{
	std::string	response;

	response  = "HTTP/1.1 " + std::to_string(_response.status)
		+ " " + reasonPhrase(_response.status) + "\r\n";
	response += "Content-Type: text/html\r\n"; // Mime?
	response += "Content-Length: " + std::to_string(_response.body.size()) + "\r\n";
	response += "Connection: " + std::string(_alive ? "keep-alive" : "close") + "\r\n";
	// Add any extra headers set by the handler (e.g. Allow, Location).
	for (const std::pair<const std::string, std::string>& header : _response.headers)
		response += header.first + ": " + header.second + "\r\n";
	response += "\r\n";
	response += _response.body;
	return (response);
}

void Client::setListenFd(int listenFd)
{
	_listenFd = listenFd;
}

int Client::getListenFd() const
{
	return (_listenFd);
}

int Client::getFd() const
{
	return (_clientFd);
}
void Client::setTime()
{
	_time = time(0);
	if (_time < 0)
		perror("Time retrieval failed");
}
int Client::getTime()
{
	return (_time);
}

int Client::checkTime() const
{
	if (time(0) - _time > TIMEOUT)
		return (-1);
	return (0);
}
