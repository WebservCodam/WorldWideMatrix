#include "Cgi.hpp"

/**
 * @brief Builds the NULL-terminated char* array that execve expects.
 *
 * execve wants argv and envp as arrays of char* ending in NULL.
 * Our text lives in std::string objects that own the bytes,
 * so this stores a pointer to each string's first character in `out`, then appends a NULL terminator.
 * The pointers stay valid only while `in` is alive and unchanged.
 *
 * @param in   The strings that own the text (the argv or env entries).
 * @param out  Filled with one char* per string, followed by a trailing NULL.
 */
static void	toCharPtrs(std::vector<std::string>& in, std::vector<char*>& out)
{
	for (std::string& str : in)
		out.push_back(&str[0]);
	out.push_back(nullptr);
}

/**
 * @brief Returns the value of a request header, or "" if it is absent.
 *
 * Header keys are stored lowercased by the parser, so the lookup is case-sensitive
 * and `key` must be passed in lowercase (e.g. "content-type").
 *
 * @param request  The parsed request whose headers are searched.
 * @param key      The lowercased header name to look up.
 * @return The header value, or an empty string if the header is not present.
 */
static std::string	header(const HttpRequest& request, const std::string& key)
{
	std::map<std::string, std::string>::const_iterator it = request.headers.find(key);
	if (it == request.headers.end())
		return ("");
	return (it->second);
}

Cgi::Cgi(const Location& location, const HttpRequest& request, const std::string& scriptPath, const std::string& serverName, const std::string& serverPort)
{
	buildArgv(location, scriptPath);
	buildEnv(request, scriptPath, serverName, serverPort);
	toCharPtrs(_argvStrings, _argv);
	toCharPtrs(_envStrings, _envp);
}

void	Cgi::buildArgv(const Location& location, const std::string& scriptPath)
{
	// The interpreter is used only when the requested file matches the
	// location's CGI extension:  argv = [interpreter, scriptPath]
	// (e.g. [/usr/bin/python3, foo.py]). Any other executable file in the
	// location runs on its own:  argv = [scriptPath]  (e.g. cgi_tester).
	const std::string&	ext = location.cgiExtension;
	bool				matchesExt = !ext.empty()
		&& scriptPath.size() >= ext.size()
		&& scriptPath.compare(scriptPath.size() - ext.size(), ext.size(), ext) == 0;

	if (!location.cgiInterpreter.empty() && matchesExt)
		_argvStrings.push_back(location.cgiInterpreter);
	_argvStrings.push_back(scriptPath);
}

void	Cgi::buildEnv(const HttpRequest& request, const std::string& scriptPath, const std::string& serverName, const std::string& serverPort)
{
	_envStrings.push_back("GATEWAY_INTERFACE=CGI/1.1");
	_envStrings.push_back("SERVER_PROTOCOL=HTTP/1.1");
	_envStrings.push_back("REDIRECT_STATUS=200");
	_envStrings.push_back("REQUEST_METHOD=" + request.method);
	_envStrings.push_back("SCRIPT_NAME=" + request.uri);
	_envStrings.push_back("SCRIPT_FILENAME=" + scriptPath);
	_envStrings.push_back("PATH_INFO=" + scriptPath);
	_envStrings.push_back("QUERY_STRING=" + request.query);
	_envStrings.push_back("CONTENT_TYPE=" + header(request, "content-type"));
	_envStrings.push_back("CONTENT_LENGTH=" + std::to_string(request.body.size()));
	_envStrings.push_back("SERVER_NAME=" + serverName);
	_envStrings.push_back("SERVER_PORT=" + serverPort);
}

char**	Cgi::getArgv()
{
	return (_argv.data());
}

char**	Cgi::getEnvp()
{
	return (_envp.data());
}
