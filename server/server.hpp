#pragma once

# include <sys/types.h>
# include <sys/socket.h>
# include <unistd.h>
# include <stdlib.h>
# include <netinet/in.h>	//For sockaddr_in (Internet domain sockets)
# include <string.h>
# include <iostream>
# include <string>

# define PORT 8080
# define NBR_OF_CONNECTIONS 10

class	Server
{
	public:
		int					_fd;
		int					_newSocket;
		long				_valRead;
		struct sockaddr_in	_address;
		int					_addrLen;

};
