/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/02 14:46:40 by rkaras        #+#    #+#                 */
/*   Updated: 2025/09/12 15:46:41 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"
#include "ListeningSocket.hpp"
#include "Server.hpp"

int main()
{
	Server server(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY, BIND, 10);
	server.launch();
	
}