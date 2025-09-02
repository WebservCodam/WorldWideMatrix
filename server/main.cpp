#include "server.hpp"

int	main(void)
{
	Server	server;

	server._addrLen = sizeof(server._address);

	char	*hello = "Hello from the server";

	// Creating socket file descriptor
	if ((server._fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("In socket");
		exit(EXIT_FAILURE);
	}

	//	Initialize struct sockaddr_in
	server._address.sin_family = AF_INET; 			//	Always set to AF_INET for IPv4 addresses
	server._address.sin_addr.s_addr = htonl(INADDR_ANY);	//	0.0.0.0 (Stored in Network Byte Order)
	server._address.sin_port = htons(PORT);			//	HTONS transform this number to Network Byte Order
	memset(server._address.sin_zero, '\0', sizeof server._address.sin_zero);

	int opt = 1;
	setsockopt(server._fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	//	Bind socket to port
	if (bind(server._fd, (struct sockaddr *) &server._address, sizeof(server._address)) < 0)
	{
		perror("In bind");
		exit(EXIT_FAILURE);
	}

	//	Listen for connections
	if (listen(server._fd, NBR_OF_CONNECTIONS) < 0)
	{
		perror("In listen");
		exit(EXIT_FAILURE);
	}

	//	Accept incoming connections. Read and write from/into the connection socket.
	while (true)
	{
		std::cout << "\n-------- Waiting for a new connection --------\n" << std::endl;

		//	Accept incoming connection
		if ((server._newSocket = accept(server._fd, (struct sockaddr *) &server._address, (socklen_t *) &server._addrLen)) < 0)
		{
			perror("In accept");
			exit(EXIT_FAILURE);			
		}

		//	Create buffer variable and initialize it to NULL
		char	buffer[30000] = {0};
		//	Read from the socket that connected to the server
		server._valRead = read(server._newSocket, buffer, 30000);
		//	Print what the server received?
		std::cout << buffer << std::endl;
		//	Now send a message to the connected socket
		write(server._newSocket, hello, strlen(hello));
		std::cout << "\n-------- Hello message sent --------\n" << std::endl;
		close(server._newSocket);
	}

	close(server._fd);

	return (0);
}

