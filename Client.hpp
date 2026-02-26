/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Client.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/12 14:00:27 by vknape        #+#    #+#                 */
/*   Updated: 2026/02/26 15:42:09 by rkaras        ########   odam.nl         */
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
		std::string response;
		
		
		Client(int fd);
		~Client();
	
		int GetFd() const;
		void SetTime();
		int GetTime();
		int CheckTime() const;
		
};