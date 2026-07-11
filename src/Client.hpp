#pragma once

#include "utils.hpp"
#include "httpparser/HttpParser.hpp"

class	Server;

struct HttpResponse
{
	int									status = 0;
	std::map<std::string, std::string>	headers;
	std::string							body;
	std::string							contentType = "text/html";	// Overridden per file by its MIME type.
};

std::string	reasonPhrase(int status);
std::string	defaultErrorPage(int status);

class Client
{
	private:
	
	public:
		const int 			_clientFd;
		int					_listenFd;
		int					_cgiFdIn = -1;
		int					_cgiFdOut = -1;
		int 				_time;
		int					_timeCgi;
		bool 				_alive = false;		// Keep-alive requested by the client (set by the parser).
		bool 				_mustClose = false;	// Server-side override: close this connection once the current response is sent.
		bool				_busy = false;		// True from the moment a full request is parsed until its response is fully flushed by connectOut; blocks connectIn from processing further buffered bytes in the meantime (it still keeps reading them into _buf).
		int					_readstate = 0;
		int					_parseready = 0;
		int 				_content_length = 0;
		std::string			_buf;
		HttpResponse		_response;
		HttpRequest			_request;
		std::string			_writeBuf;		// Serialized response; empty until built.
		size_t				_bytesSent = 0;	// Progress into _writeBuf across writes.
		size_t				_cgiBodySent = 0;
		std::string			_cgiOutput;
		bool				_cgiInputFailed = false;

		Client(int fd);
		~Client();

		std::string	serializeResponse();
		void		setListenFd(int listenFd);
		int			getListenFd() const;
		int			getFd() const;
		void		setTime();
		int			getTime();
		int			checkTime() const;
		void		setTimeCgi();
		int			getTimeCgi();
		int			checkTimeCgi() const;
};