/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Socket.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/02 12:38:35 by rkaras        #+#    #+#                 */
/*   Updated: 2025/09/02 15:23:25 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket(int domain, int service, int protocol, int port, u_long interface, int flag)
{
	//define addresss structure
	_address.sin_family = domain;
	_address.sin_addr.s_addr = htonl(interface);
	_address.sin_port = htons(port);
	
	//create a socket
	_serverFd = socket(domain, service, protocol);
	connectionValid(_serverFd);

	//bind or connect the socket
	_connection = ConnectBind(_serverFd, _address, flag);
	connectionValid(_connection);
}

int Socket::ConnectBind(int serverFd, struct sockaddr_in address, int flag)
{
	if (flag == BIND)
		return (bind(serverFd, (struct sockaddr *)&address,sizeof(address)));
	else if (flag == CONNECT)
		return (connect(serverFd, (struct sockaddr *)&address, sizeof(address)));
	return (-1);
}

void Socket::connectionValid(int value)
{
	if (value < 0)
	{
		perror("Failed to connect");
		exit(EXIT_FAILURE);
	}
}

struct sockaddr_in Socket::getAddress()
{
	return (_address);
}

int Socket::getServerFd()
{
	return (_serverFd);
}

int Socket::getConnection()
{
	return (_connection);
}