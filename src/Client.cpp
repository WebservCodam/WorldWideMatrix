/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Client.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/12 14:12:28 by vknape        #+#    #+#                 */
/*   Updated: 2026/02/17 14:06:33 by lprieri       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"

Client::Client(int fd) : _fd(fd), _time(0), _alive(false) {setTime();};

// Client::~Client() {std::cout << "destructed: " << _fd << std::endl;};

Client::~Client() {close(_fd);};

int Client::getFd() const
{
	return (_fd);
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
