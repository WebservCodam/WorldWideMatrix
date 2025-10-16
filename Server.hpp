/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vknape <vknape@student.codam.nl>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 15:04:10 by vknape            #+#    #+#             */
/*   Updated: 2025/10/16 13:21:21 by vknape           ###   ########.fr       */
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
		
		void close_client(int fd);
		void check_health();
		void add_fd_map(int client_fd);
		void connect_new();
		void connect_in(int client_fd);
		void connect_out(int client_fd);
		void print_buffers();
};
