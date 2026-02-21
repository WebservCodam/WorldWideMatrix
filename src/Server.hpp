/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/15 15:04:10 by vknape        #+#    #+#                 */
/*   Updated: 2026/01/26 13:17:10 by lprieri       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"
#include "../parser/HttpParser.hpp"
#include "configparser/Configuration.hpp"
#pragma once

class Client;

class Server
{
	private:

	public:
		Server(int epfd1);
		~Server();
		// const int server_fd;
		std::vector<int> server_fds;
		const int epfd;
		std::map<int, Client> list;
		std::vector<ServerConfig> servers;
		
		void init_server();
		void add_servers_to_epoll(int server_fd);
		void start_server();
		void close_client(int fd);
		void check_health();
		void add_fd_map(int client_fd);
		void connect_new(int server_fd);
		void connect_in(int client_fd);
		void connect_out(int client_fd);
		void print_buffers();
		void parse(int client_fd);
};
