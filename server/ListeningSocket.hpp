/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ListeningSocket.hpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/02 15:33:39 by rkaras        #+#    #+#                 */
/*   Updated: 2025/09/12 14:26:24 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef LISTENINGSOCKET_H
#define LISTENINGSOCKET_H

#include "Socket.hpp"

class ListeningSocket : public Socket
{
private:
	int _backlog;
	int _listen;
	
public:
	ListeningSocket() = delete;
	ListeningSocket(int domain, int service, int protocol, int port, u_long interface, int flag, int backlog); //domain = AF_INET, service = SOCK_STREAM, protocol = 0, port = 8080, interface = INADDR_ANY
	ListeningSocket(const ListeningSocket& other) = delete;
	ListeningSocket& operator=(const ListeningSocket& other) = delete;
	~ListeningSocket() = default;

	void listening();
	void setNonblocking(int sockFD);
};

#endif /* !LISTENINGSOCKET_H */