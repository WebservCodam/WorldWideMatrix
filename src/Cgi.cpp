#include "Cgi.hpp"

Cgi::Cgi(
	ServerConfig serverConfig,
	std::string method,
	std::string queryString,
	std::string contentType,
	std::string contentLength
	// std::string scriptName,
	// std::string pathInfo,
	// std::string serverName,
	// std::string serverPort,
)
{
	_env.push_back("REQUEST_METHOD=" + method);
	_env.push_back("QUERY_STRING=" + queryString);
	_env.push_back("CONTENT_TYPE=" + contentType);
	_env.push_back("CONTENT_LENGTH=" + contentLength);
	// _env.push_back("SCRIPT_NAME=" + ); // Create function to retrieve the looked file 
	// _env.push_back("PATH_INFO=" + );
	// _env.push_back("SERVER_NAME=" + serverConfig.getServerName());
	// _env.push_back("SERVER_PORT=" + );
	_env.push_back("SERVER_PROTOCOL=HTTP/1.1");
	_env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	_env.push_back("REDIRECT_STATUS=200");
}
