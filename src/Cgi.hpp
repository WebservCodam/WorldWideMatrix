#pragma once

#include <string>
#include <vector>
#include "configparser/Configuration.hpp"

#define WRITE_END 1
#define READ_END 0

class	Cgi
{
	private:
		std::vector<std::string>	_env;


	public:
		Cgi(
			ServerConfig serverConfig,
			std::string method,
			std::string queryString,
			std::string contentType,
			std::string contentLength,
			std::string scriptName,
			std::string pathInfo,
			std::string serverName,
			std::string serverPort
			);


};
