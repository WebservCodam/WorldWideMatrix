#pragma once

#include <string>
#include <vector>
#include "configparser/ConfigTypes.hpp"
#include "httpparser/HttpParser.hpp"

// Turns a request + location into the argv and envp arrays execve needs.
class	Cgi
{
	private:
		std::vector<std::string>	_argvStrings;	// owns the argv text
		std::vector<std::string>	_envStrings;	// owns the env text
		std::vector<char*>			_argv;			// points into _argvStrings, NULL-terminated
		std::vector<char*>			_envp;			// points into _envStrings, NULL-terminated

		void	buildArgv(const Location& location, const std::string& scriptPath);
		void	buildEnv(const HttpRequest& request, const std::string& scriptPath,	const std::string& serverName, const std::string& serverPort);

	public:
		Cgi(const Location& location,
			const HttpRequest& request,
			const std::string& scriptPath,
			const std::string& serverName,
			const std::string& serverPort);

		char**	getArgv();
		char**	getEnvp();
};
