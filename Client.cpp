/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vknape <vknape@student.codam.nl>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 14:12:28 by vknape            #+#    #+#             */
/*   Updated: 2025/10/16 14:02:32 by vknape           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"

Client::Client(int fd) : _fd(fd), _time(0), _alive(false) {SetTime();};

Client::~Client() {std::cout << "destructed: " << _fd << std::endl;};

Client::~Client() {close(_fd);};

int Client::GetFd() const
{
	return (_fd);
}
void Client::SetTime()
{
	_time = time(0);
	if (_time < 0)
		perror("Time retrieval failed");
}
int Client::GetTime()
{
	return (_time);
}

int Client::CheckTime() const
{
	if (time(0) - _time > TIMEOUT)
		return (-1);
	return (0);
}
