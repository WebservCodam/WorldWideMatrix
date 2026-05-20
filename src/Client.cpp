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
};

Client::~Client() 
{
	close(_clientFd);
}

std::string	Client::serializeResponse()
{
	return (_response.headers.at("header") + _response.body);
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
