/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vknape <vknape@student.codam.nl>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 14:00:27 by vknape            #+#    #+#             */
/*   Updated: 2025/09/18 13:51:09 by vknape           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #include <string>
// #include <vector>
// #include <map>
// #include <iostream>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <sys/epoll.h>
// #include <unistd.h>
#include "utils.hpp"

#pragma once

class Server;

class Client
{
	private:
	
	public:
		const int _fd;
		int _time;
		bool _alive = false;
		int readstate = 0;
		int	parseready = 0;
		int content_length = 0;
		std::string _buf;
		
		Client(int fd);
		~Client();
	
		int GetFd() const;
		void SetTime();
		int GetTime();
		int CheckTime() const;
		
};


int add_fd_map(int client_fd, Server& server);