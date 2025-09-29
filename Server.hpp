/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vknape <vknape@student.codam.nl>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 15:04:10 by vknape            #+#    #+#             */
/*   Updated: 2025/09/18 13:47:38 by vknape           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

#pragma once

class Client;

class Server
{
	private:

	public:
		Server(int server_fd1, int epfd1);
		~Server();
		const int server_fd;
		const int epfd;
		std::map<int, Client> list;
};

int connect_new(Server& server);
int connect_in(int client_fd, Server& server);
int connect_out(int client_fd, Server& server);
void print_buffers(Server& server);
void check_health(Server& server);