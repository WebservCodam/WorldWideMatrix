/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Socket.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/02 12:55:22 by rkaras        #+#    #+#                 */
/*   Updated: 2025/09/02 14:24:06 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>

# define BIND 0
# define CONNECT 1

class Socket
{
private:
	struct sockaddr_in _address;
	int _serverFd;
	int _connection;
	
public:
	Socket() = delete;
	Socket(int domain, int service, int protocol, int port, u_long interface, int flag); //domain = AF_INET, service = SOCK_STREAM, protocol = 0, port = 8080, interface = INADDR_ANY
	Socket(const Socket& other) = delete;
	Socket& operator=(const Socket& other) = delete;
	~Socket() = default;

	int ConnectBind(int serverFd, struct sockaddr_in address, int flag);

	void connectionValid(int value);

	struct sockaddr_in getAddress();
	int getServerFd();
	int getConnection();
};

#endif /* !SOCKET_H */