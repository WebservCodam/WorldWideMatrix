#pragma once

#include "utils.hpp"
#include "httpparser/HttpParser.hpp"

class	Server;

struct HttpResponse
{
	int									status;
	std::map<std::string, std::string>	headers;
	std::string							body;
	std::string							contentType = "text/html";	// Overridden per file by its MIME type.
};

// Maps an HTTP status code to its reason phrase (e.g. 404 -> "Not Found").
std::string	reasonPhrase(int status);
// Builds a self-contained HTML error page for `status`, using its reason phrase.
// Served when no error_page file is configured or it can't be read.
std::string	defaultErrorPage(int status);

class Client
{
	private:
	
	public:
		const int 			_clientFd;
		int					_listenFd;
		int 				_time;
		bool 				_alive = false;		// Keep-alive requested by the client (set by the parser).
		bool 				_mustClose = false;	// Server-side override: close this connection once the current response is flushed.
		int					_readstate = 0;
		int					_parseready = 0;
		int 				_content_length = 0;
		std::string			_buf;
		HttpResponse		_response;
		HttpRequest			_request;
		std::string			_writeBuf;		// Serialized response; empty until built.
		size_t				_bytesSent = 0;	// Progress into _writeBuf across writes.

		Client(int fd);
		~Client();

		std::string	serializeResponse(); // This creates the ready to send response that'll be written to the socket.
		void		setListenFd(int listenFd);
		int			getListenFd() const;
		int			getFd() const;
		void		setTime();
		int			getTime();
		int			checkTime() const;
};
