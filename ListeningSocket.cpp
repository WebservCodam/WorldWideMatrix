/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ListeningSocket.cpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/02 15:40:05 by rkaras        #+#    #+#                 */
/*   Updated: 2025/09/02 15:47:20 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "ListeningSocket.hpp"

ListeningSocket::ListeningSocket(int domain, int service, int protocol, int port, u_long interface, int flag, int backlog) : Socket(domain, service, protocol, port, interface, flag)
{
	_backlog = backlog;
	listening();
	connectionValid(_listen);
}

void ListeningSocket::listening()
{
	_listen = listen(getServerFd(), _backlog);
}