#include "client.hpp"

int	main(void)
{
	Client	client;
	char	*hello = "Hello from the client";
	char	buffer[1024] = {0};

	if ((client._sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Creating socket");
		exit (EXIT_FAILURE);
	}

	//	Init struct sockaddr_in
	memset(&client._servAddr, 0, sizeof(client._servAddr));
	client._servAddr.sin_family = AF_INET;
	client._servAddr.sin_port = htons(PORT);
	//	Convert IPv4 address from text to binary form
	if (inet_pton(AF_INET, "127.0.0.1", &client._servAddr.sin_addr) <= 0)
	{
		perror("Invalid address / Address not supported");
		exit(EXIT_FAILURE);
	}

	//	Connect to server
	if (connect(client._sock, (struct sockaddr *) &client._servAddr, sizeof(client._servAddr)) < 0)
	{
		perror("Connection failed");
		exit(EXIT_FAILURE);
	}

	//	Send message to the socket
	send(client._sock, hello, strlen(hello), 0);
	std::cout << "Hello message sent" << std::endl;
	//	Read message from the server
	client._valRead = read(client._sock, buffer, 1024);
	std::cout << "From server: " << buffer << std::endl;

	// Close the client socket after you're done reading
    close(client._sock);
	
	return (0);	
}
