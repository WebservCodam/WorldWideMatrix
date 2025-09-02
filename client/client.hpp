#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

# define PORT 8080

class	Client
{
	public:
		int					_sock;
		long				_valRead;
		struct sockaddr_in	_servAddr;

};
