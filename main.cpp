/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/02 14:46:40 by rkaras        #+#    #+#                 */
/*   Updated: 2025/09/02 15:03:32 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

int main()
{
	Socket server(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY, BIND);
	Socket client(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY, CONNECT);
}