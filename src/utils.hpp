/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vknape <vknape@student.codam.nl>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 13:53:29 by vknape            #+#    #+#             */
/*   Updated: 2026/06/26 14:57:43 by vknape           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <sstream>
#include <sys/epoll.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <ctime>
#include <stdexcept>
#include <vector>
#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include <cstring>

#define MAXFD 1024
#define TIMEOUT 15
#define TIMEOUT_CGI 10
#define TIMEOUT_KEEP_ALIVE 75

#pragma once

void	setNonBlocking(int fd);
void	initServer(int& listenFd, int& epfd);
int		createSocket(const char* ip, const char* port);