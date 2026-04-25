/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Client.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/12 14:00:27 by vknape        #+#    #+#                 */
/*   Updated: 2026/02/26 16:05:48 by lprieri       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// Pragma once should be at the top to avoid circular dependencies

// #include <string>
// #include <vector>
// #include <map>
// #include <iostream>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <sys/epoll.h>
// #include <unistd.h>
#include "utils.hpp"
#include "../parser/HttpParser.hpp"

class	Server;

class Client
{
	private:
	
	public:
		const int 			_fd;
		int 				_time;
		bool 				_alive = false;
		int					_readstate = 0;
		int					_parseready = 0;
		int 				_content_length = 0;
		std::string			_buf;
		std::string			_response;
		unsigned long long	_maxBodySize;
		HttpRequest			_request;

		Client(int fd);
		~Client();
	
		int		getFd() const;
		void	setTime();
		int		getTime();
		int		checkTime() const;
};
