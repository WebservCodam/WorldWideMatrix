/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/01 10:59:15 by vknape        #+#    #+#                 */
/*   Updated: 2026/05/20 10:46:08 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"

Server::Server(const ServerConfig& serverConfig): _serverConfig(serverConfig) {}

void	Server::addListenFd(int listenFd)
{
	_listenFds.push_back(listenFd);
}

// Reads the entire file at `path` into `out`.
// Returns false if the file can't be opened, true on success.
static bool	loadFile(const std::string& path, std::string& out)
{
	std::ifstream	file(path);

	if (!file.is_open())
		return (false);

	std::stringstream	buffer;
	buffer << file.rdbuf();
	out = buffer.str();
	return (true);
}

// Returns true if `path` exists and is a directory.
static bool	isDirectory(const std::string& path)
{
	struct stat	st;

	return (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
}

// Fills `res` with an error page for `code`: tries the error page file
// configured for that code, and falls back to a built-in HTML string.
void	Server::serveErrorPage(HttpResponse& res, int code)
{
	res.status = code;
	try
	{
		ErrorPage	errorPage = _serverConfig.getErrorPage(code);
		if (loadFile(errorPage.URI, res.body))
			return ;
	}
	catch (const std::exception&)
	{
	}
	res.body = "<html><body><h1>" + std::to_string(code)
		+ " Error</h1></body></html>";
}

// Appends `method` to a comma-separated list, inserting ", " when needed.
static void	appendMethod(std::string& list, const std::string& method)
{
	if (!list.empty())
		list += ", ";
	list += method;
}

// Builds the "Allow" header value from the methods the location permits,
// e.g. "GET, POST".
static std::string	allowedMethods(const Location& location)
{
	std::string	list;

	if (location.getMethod)
		appendMethod(list, "GET");
	if (location.postMethod)
		appendMethod(list, "POST");
	if (location.deleteMethod)
		appendMethod(list, "DELETE");
	return (list);
}

void	Server::handleRequest(Client& client)
{
	HttpResponse&	res = client._response;

	std::cout << "DEBUG - in handleRequest" << std::endl;
	try
	{
		const std::string&	uri = client._request.uri;
		const Location&		location = _serverConfig.getLocation(uri);

		// Is the request method allowed for this location? If not, 405 with
		// an Allow header listing the methods that are permitted.
		const std::string&	method = client._request.method;
		bool				allowed = (method == "GET" && location.getMethod)
			|| (method == "POST" && location.postMethod)
			|| (method == "DELETE" && location.deleteMethod);
		if (!allowed)
		{
			res.headers["Allow"] = allowedMethods(location);
			serveErrorPage(res, 405);
			return ;
		}


		// Reject bodies larger than the location's limit (0 == no limit).
		if (location.maxBodySize != 0 && client._request.body.size() > location.maxBodySize)
		{
			std::cout << "DEBUG2 - maxBodySize: " << location.maxBodySize << std::endl;
			serveErrorPage(res, 413);
			return ;
		}

		// Strip the matched location prefix off the URI, then join the rest
		// onto the location's directory to get the real filesystem path.
		std::string	prefixLocation = (location.name == "/") ? "/" : "/" + location.name;
		std::string	remainder = uri.substr(prefixLocation.size());
		std::string	fsPath = joinPath(location.dirPath, remainder);

		// A directory request serves the location's index; otherwise serve the file itself.
		if (isDirectory(fsPath))
		{
			if (loadFile(location.indexPath, res.body))
				res.status = 200;
			else
				// TODO: if the index is missing and location.autoindex is on,
				// list the directory contents instead of returning 404.
				serveErrorPage(res, 404);
		}
		else if (loadFile(fsPath, res.body))
			res.status = 200;
		else
			serveErrorPage(res, 404);
		std::cout << "DEBUG: fsPath is: " + fsPath << std::endl;
	}
	catch (const std::exception&)
	{
		serveErrorPage(res, 404);
	}
}
