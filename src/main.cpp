#include "Client.hpp"
#include "utils.hpp"
#include "Server.hpp"
#include "Webserv.hpp"

#include "configparser/Configuration.hpp"
#include "configparser/ServerConfig.hpp"

void	printErrorAndExit(const std::string& msg, int errorCode)
{
	std::cerr << msg << std::endl;
	exit(errorCode);
}

void	initialize(int argc, char **argv, std::vector<ServerConfig>& configurations)
{
	if (argc != 2)
		printErrorAndExit("Error: Expecting an input file.", EXIT_FAILURE);

	std::ifstream	file(argv[1]);
	if (!file)
		printErrorAndExit("Error: Could not open file.", EXIT_FAILURE);

	std::stringstream	buffer;
	buffer << file.rdbuf();
	std::string input = buffer.str();
	
	try
	{
		std::unique_ptr<ConfigFile> ast = Parser(input).parse();
		configurations = ast->createServers();
	}
	catch (const ConfigError& e)
	{
		printErrorAndExit(e.what(), EXIT_FAILURE);
	}
	catch (const std::exception& e)
	{
		printErrorAndExit(std::string("Unexpected error\n") + e.what(), EXIT_FAILURE);
	}
}

int main(int argc, char** argv)
{
	std::vector<ServerConfig>	configurations;

	initialize(argc, argv, configurations);
	while (true)
	{
		try
		{
			int	epfd;

			epfd = epoll_create(EPOLL_NBR_EVENTS);

			if (epfd < 0)
				throw std::runtime_error("Failed to create epoll fd");

			Webserv	webserver(epfd);
			
			webserver.setServerConfigs(configurations);

			webserver.initWebserv();
			webserver.startServers();

		}	catch (const std::runtime_error& e) {
				std::cout << "Runtime error: " << e.what() << std::endl;
				exit(EXIT_FAILURE);
		}	catch (const std::exception& e) {
				std::cout << "Exception: " << e.what() << std::endl;
		}
		exit(0);
	}
}
