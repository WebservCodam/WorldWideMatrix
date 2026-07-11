#include "Client.hpp"
#include "utils.hpp"
#include "Server.hpp"
#include "Webserv.hpp"

#include "configparser/Configuration.hpp"
#include "configparser/ServerConfig.hpp"

volatile bool g_run_server = true;

void	initialize(int argc, char **argv, std::vector<ServerConfig>& configurations);
void	printErrorAndExit(const std::string& msg, int errorCode);

void	signalHandler(int sig)
{
	(void)sig;
	g_run_server = false;
}

int main(int argc, char** argv)
{
	std::vector<ServerConfig>	configurations;

	initialize(argc, argv, configurations);
	int restart = 0;
	while (g_run_server)
	{
		if (restart > 4)
		{
			std::cout << "Internal issue could not be resolved, proper restart needed" << std::endl;
			exit(1);
		}
		std::cout << "Starting webserv" << std::endl;
		try
		{
			int	epfd;

			epfd = epoll_create(EPOLL_NBR_EVENTS);

			if (epfd < 0)
				throw std::runtime_error("Failed to create epoll fd");

			Webserv	webserver(epfd);

			webserver.setServerConfigs(configurations);

			signal(SIGINT, signalHandler);
			signal(SIGPIPE, SIG_IGN);
			webserver.initWebserv();
			webserver.startServers();
		}	catch (const std::runtime_error& e) {
				std::cout << "Runtime error: " << e.what() << std::endl;
		}	catch (const std::exception& e) {
				std::cout << "Exception: " << e.what() << std::endl;
		}
		restart++;
	}
	std::cout << "Sigint received" << std::endl;
}

void	initialize(int argc, char **argv, std::vector<ServerConfig>& configurations)
{
	if (argc > 2)
		printErrorAndExit("Usage: ./webserv [configuration file]", EXIT_FAILURE);

	std::string configPath = (argc == 2) ? argv[1] : DEFAULT_CONF;
	if (argc == 1)
		std::cout << "No configuration file provided, using " << configPath << std::endl;

	std::ifstream	file(configPath.c_str());
	if (!file)
		printErrorAndExit("Error: Could not open file: " + configPath, EXIT_FAILURE);

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

void	printErrorAndExit(const std::string& msg, int errorCode)
{
	std::cerr << msg << std::endl;
	exit(errorCode);
}
