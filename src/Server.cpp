/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/01 10:59:15 by vknape        #+#    #+#                 */
/*   Updated: 2026/06/05 19:59:51 by lprieri       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"

#include <cctype>	// std::tolower in mimeType()

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

// Maps a file's extension to its MIME type for the Content-Type header.
// Matching is case-insensitive.
// Unknown extensions fall back to a generic content-type.
static std::string	mimeType(const std::string& path)
{
	static const std::map<std::string, std::string>	types = {
		{".html", "text/html"}, {".htm", "text/html"},
		{".css", "text/css"}, {".js", "text/javascript"},
		{".json", "application/json"}, {".txt", "text/plain"},
		{".png", "image/png"}, {".gif", "image/gif"},
		{".jpg", "image/jpeg"}, {".jpeg", "image/jpeg"},
		{".svg", "image/svg+xml"}, {".ico", "image/x-icon"},
		{".pdf", "application/pdf"},
	};

	// Only look at the last path component so dots in directory names
	// (e.g. "/a.b/index") aren't mistaken for an extension.
	std::string::size_type	slash = path.find_last_of('/');
	std::string::size_type	dot = path.find_last_of('.');
	if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
		return ("application/octet-stream");

	std::string	ext = path.substr(dot);
	for (char& c : ext)
		c = std::tolower(static_cast<unsigned char>(c));

	std::map<std::string, std::string>::const_iterator	it = types.find(ext);
	if (it != types.end())
		return (it->second);
	return ("application/octet-stream");
}

// Fills `res` with an error page for `code`: tries the error page file
// configured for that code, and falls back to a generated default page.
void	Server::serveErrorPage(HttpResponse& res, int code)
{
	res.status = code;
	res.contentType = "text/html";
	try
	{
		ErrorPage	errorPage = _serverConfig.getErrorPage(code);
		if (loadFile(errorPage.URI, res.body))
			return ;
	}
	catch (const std::exception&)
	{
	}
	res.body = defaultErrorPage(code);
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

// Applies a `return` directive: a 3xx code redirects (page -> Location
// header); any other code serves the error page for that status.
void	Server::serveReturn(HttpResponse& res, const ReturnPage& ret)
{
	res.status = ret.code;
	if (ret.code >= 300 && ret.code < 400)
	{
		if (!ret.uri.empty())
			res.headers["Location"] = ret.uri;
	}
	else
		serveErrorPage(res, ret.code);
}

void	Server::servePost(HttpResponse& res, const std::string& body, const Location& location, const std::string& remainder)
{
	if (remainder.empty() || remainder == "/")
	{
		serveErrorPage(res, 400);
		return ;
	}

	if (access(location.uploadPath.c_str(), W_OK) != 0)
	{
		serveErrorPage(res, 403);
		return ;
	}

	std::string		uploadPath = joinPath(location.uploadPath, remainder);
	std::ofstream	out(uploadPath, std::ios::binary | std::ios::trunc);
	if (!out.is_open())
	{
		serveErrorPage(res, 500);
		return ;
	}
	out.write(body.data(), static_cast<std::streamsize>(body.size()));
	if (!out)
	{
		serveErrorPage(res, 500);
		return ;
	}
	res.status = 201;
}

void	Server::handleRequest(Client& client)
{
	HttpResponse&	res = client._response;

	std::cout << "DEBUG - in handleRequest" << std::endl;
	try
	{
		const std::string&	uri = client._request.uri;
		const Location&		location = _serverConfig.getLocation(uri);

		// A `return` directive short-circuits everything else.
		if (location.returnPage.code != -1)
		{
			serveReturn(res, location.returnPage);
			return ;
		}

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

		if (method == "POST")
		{
			servePost(res, client._request.body, location, remainder);
			return ;
		}

		// A directory request serves the location's index; otherwise serve the file itself.
		if (isDirectory(fsPath))
		{
			if (loadFile(location.indexPath, res.body))
			{
				res.status = 200;
				res.contentType = mimeType(location.indexPath);
			}
			else
				// TODO: if the index is missing and location.autoindex is on,
				// list the directory contents instead of returning 404.
				serveErrorPage(res, 404);
		}
		else if (loadFile(fsPath, res.body))
		{
			res.status = 200;
			res.contentType = mimeType(fsPath);
		}
		else
			serveErrorPage(res, 404);
		std::cout << "DEBUG: fsPath is: " + fsPath << std::endl;
	}
	catch (const std::exception&)
	{
		serveErrorPage(res, 404);
	}
}
