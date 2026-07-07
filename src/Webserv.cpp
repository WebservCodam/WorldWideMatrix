#include "Webserv.hpp"
#include "Client.hpp"

Webserv::~Webserv()
{
	// Close every listening socket we opened. They're deduplicated in
	// _listenFdToServers (one key per host:port), so each fd is closed exactly once.
	for (std::map<int, std::vector<Server*>>::iterator it = _listenFdToServers.begin(); it != _listenFdToServers.end(); ++it)
		close(it->first);
	for (std::map<pid_t, int>::iterator it = _cgiPid.begin(); it != _cgiPid.end(); it++)
		kill(it->first, SIGKILL);
	// for (std::map<int, int>::iterator it = _cgiFdToClientIn.begin(); it != _cgiFdToClientIn.end(); it++)
	// 	close(it->first);
	// for (std::map<int, int>::iterator it = _cgiFdToClientOut.begin(); it != _cgiFdToClientOut.end(); it++)
	// 	close(it->first);
	// Client fds are closed by ~Client when _clients is destroyed.
	close(_epfd);
}

void	Webserv::initWebserv()
{
	std::map<std::string, int>	hostPortToFd;	// Collapses duplicate host:port across servers onto a single socket.

	_servers.reserve(_serverConfigs.size());
	for (const ServerConfig& serverConfig : _serverConfigs)
	{
		_servers.push_back(Server(serverConfig));
		Server&	server = _servers.back();
		const std::vector<ListenDirective>&	listenDirs = serverConfig.getListenDirectives();

		for (const ListenDirective& listenDir : listenDirs)
		{
			int	listenFd = getOrCreateListenSocket(listenDir, hostPortToFd);
			_listenFdToServers[listenFd].push_back(&server);
		}
	}
}

// Returns the listening socket for listenDir's host:port, creating and registering
// it the first time that address is seen and reusing it for every later server that
// shares it (so virtual hosts on the same host:port share one socket).
int	Webserv::getOrCreateListenSocket(const ListenDirective& listenDir, std::map<std::string, int>& hostPortToFd)
{
	std::string								key = listenDir.address + ":" + listenDir.port;
	std::map<std::string, int>::iterator	it = hostPortToFd.find(key);

	if (it != hostPortToFd.end())
		return (it->second);

	int	listenFd = createSocket(listenDir.address.c_str(), listenDir.port.c_str());
	hostPortToFd.insert(std::make_pair(key, listenFd));
	_listenFdToPort[listenFd] = listenDir.port;
	if (!epollCtl(EPOLL_CTL_ADD, listenFd, EPOLLIN | EPOLLPRI | EPOLLHUP))
		throw std::runtime_error("Server add to epoll failed");
	return (listenFd);
}

// Registers (op == EPOLL_CTL_ADD) or updates (op == EPOLL_CTL_MOD) fd in epoll
// with the given event flags. Returns true on success, false on failure; the
// caller decides how to handle a failure (throw for listen fds, close the client otherwise).
bool	Webserv::epollCtl(int op, int fd, uint32_t events)
{
	struct epoll_event	event;

	event.events = events;
	event.data.fd = fd;
	return (epoll_ctl(_epfd, op, fd, &event) != -1);
}

void	Webserv::startServers()
{
	epoll_event	events[EPOLL_NBR_EVENTS];
	int			numEvents = 0;
	int			connections = 0;

	while (true)
	{
		numEvents = epoll_wait(_epfd, events, EPOLL_NBR_EVENTS, EPOLL_TIMEOUT);
		if (numEvents == -1)
			throw std::runtime_error("Epoll_wait failed");
		
		for (int i = 0; i < numEvents; i++)
		{
			int			eventFd = events[i].data.fd;
			uint32_t	fdEvents = events[i].events;
			bool		handled = false;

			std::cout << "DEBUG: We're in the startServers for loop." << std::endl;

			if (_listenFdToServers.find(eventFd) != _listenFdToServers.end() && (fdEvents & EPOLLIN))
			{
				connectNew(eventFd);
				connections++;
				handled = true;
			}
			if (_cgiFdToClientIn.find(eventFd) != _cgiFdToClientIn.end() && (fdEvents & EPOLLOUT))
			{
				writeToCgi(eventFd);
				handled = true;
			}
			if (_cgiFdToClientOut.find(eventFd) != _cgiFdToClientOut.end() && (fdEvents & EPOLLIN))
			{
				readFromCgi(eventFd);
				handled = true;
			}
			// else if (_cgiFdToClientOut.find(eventFd) != _cgiFdToClientOut.end() && events[i].events & EPOLLHUP)
			// {
			// 	//cgi out done
			// 	finishCgi(eventFd, client, errorCode);
			// }
			if (_clients.find(eventFd) != _clients.end() && (fdEvents & EPOLLERR))
			{
				closeAndRemoveFdFromClientList(eventFd);
				handled = true;
			}
			else
			{
				// A client fd can now be armed for EPOLLIN and EPOLLOUT at the
				// same time (reading a pipelined request while still flushing
				// the current response), so both directions are checked
				// independently rather than as else-if branches, and each
				// re-checks _clients so a close from the read side is seen
				// before we'd otherwise try to write to the same fd.
				if (_clients.find(eventFd) != _clients.end() && (fdEvents & EPOLLIN))
				{
					connectIn(eventFd);
					handled = true;
				}
				if (_clients.find(eventFd) != _clients.end() && (fdEvents & EPOLLOUT))
				{
					connectOut(eventFd);
					handled = true;
				}
			}

			if (!handled)
				throw std::runtime_error("This FD doesn't belong to a server nor a client.");
		}
		checkHealth();
		if (!g_run_server)
			return;
	}
}

void Webserv::connectNew(int listenFd)
{
	int	clientFd;

	while (true)
	{
		struct sockaddr_in	clientAddr;	// When we accept we configure this struct to match the address of the connecting peer.
		socklen_t			clientLen = sizeof(clientAddr);

		clientFd = accept(listenFd, (struct sockaddr*) &clientAddr, &clientLen);
		if (clientFd == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)	// Remember to remove these checks. Check the Edge Cases notes in Obsidian.
				return ;
			else
				throw std::runtime_error("Client accept failed");
		}

		setNonBlocking(clientFd); // Changed the function to void because in init we didn't check the return value, so now it directly throws an error from within the function.

		try
		{
			addFdToClientList(clientFd, listenFd);
		}
		catch (const std::exception& e)
		{
			std::cout << "Failed to create and add Client object: " << e.what() << std::endl;
			throw ;
		}

		// EPOLLIN: notified when the fd is ready for reading. No edge-triggered flag,
		// because it might be impossible to implement with the errno constraint.
		if (!epollCtl(EPOLL_CTL_ADD, clientFd, EPOLLIN))
		{
			closeAndRemoveFdFromClientList(clientFd);
			throw std::runtime_error("Failed to add client to epoll");
		}
	}
}

void	Webserv::addFdToClientList(int clientFd, int listenFd)
{
	std::pair<std::map<int, Client>::iterator, bool>	result;

	result = _clients.emplace(clientFd, clientFd);
	result.first->second.setListenFd(listenFd);	// The client's server is chosen per-request from _listenFdToServers once its Host header is parsed.
}

void	Webserv::closeAndRemoveFdFromClientList(int clientFd)
{
	_clients.erase(clientFd); // The Client destructor closes the fd, which also removes it from epoll.
}

void	Webserv::closeAndCleanCgi()
{
	int status;
	pid_t finished_pid;

	while ((finished_pid = waitpid(-1, &status, WNOHANG))> 0)
	{
		auto it = _cgiPid.find(finished_pid);

		if (it != _cgiPid.end())
		{
			closeCgiPipes(it->second);
			_cgiPid.erase(it);
		}
		else
			std::cout << "Reaped unknown child process, pid: " << finished_pid << std::endl;
	}
}
void	Webserv::closeCgiPipes(int clientFd)
{
	if (_clients.at(clientFd)._cgiFdIn != -1)
	{
		close(_clients.at(clientFd)._cgiFdIn);
		_cgiFdToClientIn.erase(_clients.at(clientFd)._cgiFdIn);
		_clients.at(clientFd)._cgiFdIn = -1;
	}

	if (_clients.at(clientFd)._cgiFdOut != -1)
	{
		close(_clients.at(clientFd)._cgiFdOut);
		_cgiFdToClientOut.erase(_clients.at(clientFd)._cgiFdOut);
		_clients.at(clientFd)._cgiFdOut = -1;
	}

}

// Tries to parse whatever is already sitting in the client's buffer (freshly
// received bytes, or leftovers from a pipelined request that arrived earlier)
// and, if a full request is present, routes it and serializes the response.
// Never performs a read()/recv()/write()/send() itself: it only inspects bytes
// already in memory and updates the epoll registration via epollCtl so the
// actual I/O happens the next time epoll_wait reports the fd ready.
//  - INCOMPLETE: more bytes are needed, so arm EPOLLIN.
//  - COMPLETE / ERROR: a response was built, so arm EPOLLOUT to flush it.
void	Webserv::processBufferedRequest(int clientFd)
{
	Client&		client = _clients.at(clientFd); //If unsuccesful it throws an std::out_of_range
	ParseStatus	status;

	try
	{
		status = parse(clientFd); // Branch the status received into conditionals.

		if (status == INCOMPLETE)
		{
			std::cout << "Parsing incomplete; waiting for more bytes." << std::endl;
			if (!epollCtl(EPOLL_CTL_MOD, clientFd, EPOLLIN))
			{
				perror("Epoll_ctl: switch to EPOLLIN failed");
				closeAndRemoveFdFromClientList(clientFd);
			}
			return ;
		}

		// A full request is present and we're about to build its response:
		// mark the connection busy so connectIn keeps buffering any further
		// (pipelined) bytes without acting on them until connectOut has fully
		// flushed this response.
		client._busy = true;

		if (status == ERROR)
		{
			// The parser set the error status; build its body and serve it as is instead of routing the request (which would overwrite the status).
			selectServer(client.getListenFd(), getRequestHost(client))->serveErrorPage(client._response, client._response.status);
		}
		else // COMPLETE: route the request and build the response now, while we wait for EPOLLOUT. connectOut only pushes bytes.
			selectServer(client.getListenFd(), getRequestHost(client))->handleRequest(client);

		client._writeBuf = client.serializeResponse();
		client._bytesSent = 0;

		// Keep EPOLLIN armed alongside EPOLLOUT: the client may keep sending
		// pipelined requests while we're still flushing this response; those
		// bytes get buffered (see connectIn's _busy check) and parsed once
		// connectOut clears _busy.
		if (!epollCtl(EPOLL_CTL_MOD, clientFd, EPOLLIN | EPOLLOUT))
		{
			perror("Epoll_ctl: switch to EPOLLIN|EPOLLOUT failed");
			closeAndRemoveFdFromClientList(clientFd);
		}
	}
	catch (const std::exception& e)
	{
		// An unexpected failure while building the response: notify the client with a
		// 500 page instead of crashing the server, then close once it's flushed.
		std::cerr << "Server error while handling request: " << e.what() << std::endl;
		serveError(client, 500, true);
	}
}

void Webserv::connectIn(int clientFd)
{
	char		buffer[8192] = {0};
	ssize_t		count = 0; // Bytes read.

	count = recv(clientFd, buffer, sizeof(buffer), 0);
	if (count == 0 || count == -1)
	{
		if (count == -1)
			perror("recv failed");
		closeAndRemoveFdFromClientList(clientFd);
		return ;
	}

	Client&	client = _clients.at(clientFd); //If unsuccesful it throws an std::out_of_range

	client.setTime(); // Reset time after a succesful read.
	client._buf.append(buffer, count); // Changed this line to the CPP version.

	if (client._busy)
		return ; // A response for the previous request is still being built/flushed
				 // (or a CGI is running); leave the new bytes buffered and let
				 // connectOut resume parsing once that response is fully sent.

	processBufferedRequest(clientFd);
}

// Serves `code` as an error page on the client and arms EPOLLOUT so connectOut
// flushes it. When closeConnection is true the connection is dropped after the
// page is sent (Connection: close); otherwise the client's keep-alive choice stands,
// so the connection can serve another request. Kept exception-safe: it falls back to
// a built-in page if the per-server lookup itself fails, since it runs on error paths.
void	Webserv::serveError(Client& client, int code, bool closeConnection)
{
	client._response = HttpResponse(); // Drop any half-built response so its headers don't leak.
	try
	{
		selectServer(client.getListenFd(), getRequestHost(client))->serveErrorPage(client._response, code);
	}
	catch (const std::exception&)
	{
		client._response.status = code;
		client._response.contentType = "text/html";
		client._response.body = defaultErrorPage(code);
	}

	if (closeConnection)
		client._mustClose = true;

	client._writeBuf = client.serializeResponse();
	client._bytesSent = 0;
	client._busy = true; // A response is now in flight; cleared by connectOut once it's fully sent.

	if (!epollCtl(EPOLL_CTL_MOD, client.getFd(), EPOLLIN | EPOLLOUT))
		closeAndRemoveFdFromClientList(client.getFd()); // Can't even notify the client; drop it.
}

void Webserv::connectOut(int clientFd)
{
	Client&	client = _clients.at(clientFd);

	// The response was built in connectIn; this function only sends it.
	// One write per epoll event. write() may take only part of the buffer;
	// we resume from _bytesSent when EPOLLOUT fires again.
	ssize_t	written = write(clientFd,
			client._writeBuf.data() + client._bytesSent,
			client._writeBuf.size() - client._bytesSent);
	if (written == -1)
	{
		// errno can't be checked after a write. epoll just reported the socket as writable, so -1 is a real error, not a full buffer. So we close the client.
		closeAndRemoveFdFromClientList(clientFd);
		return ;
	}
	client._bytesSent += written;
	client.setTime(); // Progress counts as activity: don't time out a slow download.

	if (client._bytesSent < client._writeBuf.size())
		return ; // Still subscribed to EPOLLOUT; we resume when the socket drains.

	// Response fully sent: reset per-response state for the next request on
	// this connection. _buf is deliberately left untouched here: if the client
	// pipelined a second request (sent it without waiting for this response),
	// its bytes are already sitting in _buf and must not be discarded.
	client._writeBuf.clear();
	client._bytesSent = 0;
	client._response = HttpResponse(); // Headers like Allow or Location must not leak into the next response.
	client._busy = false; // No response in flight anymore; connectIn/processBufferedRequest can act on buffered bytes again.

	if (client._alive == false || client._mustClose)
	{
		closeAndRemoveFdFromClientList(clientFd);
		return ;
	}

	if (!epollCtl(EPOLL_CTL_MOD, clientFd, EPOLLIN))
	{
		perror("Epoll_ctl: switch to EPOLLIN failed");
		closeAndRemoveFdFromClientList(clientFd);
	}
	return ;

	// A pipelined request may already be fully (or partially) buffered from an
	// earlier recv(). Parse what's already in memory now instead of waiting for
	// another EPOLLIN event: the client may not send more bytes if it already
	// sent everything back-to-back and is just waiting on replies.
	processBufferedRequest(clientFd);
}

void	Webserv::writeToCgi(int cgiInFd)
{
	Client&				client = _clients.at(_cgiFdToClientIn.at(cgiInFd));
	const std::string&	body = client._request.body;

	ssize_t	written = write(cgiInFd,
			body.data() + client._cgiBodySent,
			body.size() - client._cgiBodySent);
	if (written != -1)
	{
		client._cgiBodySent += written;
		if (client._cgiBodySent < body.size())
			return ;	// Partial write: stay armed for EPOLLOUT and resume.
	}
	epollCtl(EPOLL_CTL_DEL, cgiInFd, 0);
	close(cgiInFd);
	_cgiFdToClientIn.erase(cgiInFd);
	client._cgiFdIn = -1;
}

void	Webserv::readFromCgi(int cgiOutFd)
{
	Client&	client = _clients.at(_cgiFdToClientOut.at(cgiOutFd));
	char	buffer[8192];
	ssize_t	count = read(cgiOutFd, buffer, sizeof(buffer));

	if (count > 0)
	{
		client._cgiOutput.append(buffer, count);
		return ;	// Keep reading until EOF; output can arrive in pieces.
	}
	if (count == -1)
	{
		finishCgi(cgiOutFd, client, 502);
		return ;
	}
	// count == 0: the script closed stdout, so it's done.
	if (client._cgiOutput.empty())
		finishCgi(cgiOutFd, client, 502);	// A CGI that printed nothing is a failure.
	else
	{
		buildResponseFromCgi(client);
		finishCgi(cgiOutFd, client, 0);
	}
}

void Webserv::buildResponseFromCgi(Client& client)
{
    HttpResponse&      res = client._response;
    const std::string& out = client._cgiOutput;

    res.status = 200;  // CGI default, unless a Status header overrides it

    // Find the blank line that separates the CGI headers from the body.
    // Accept both CRLF (spec) and bare LF (what many scripts actually emit).
    size_t sep = out.find("\r\n\r\n");
    size_t sepLen = 4;
    if (sep == std::string::npos)
    {
        sep = out.find("\n\n");
        sepLen = 2;
    }
    if (sep == std::string::npos)   // no header block: the whole output is the body
    {
        res.body = out;
        return;
    }

    std::string        headerBlock = out.substr(0, sep);
    res.body = out.substr(sep + sepLen);

    std::istringstream stream(headerBlock);
    std::string        line;
    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')   // tolerate CRLF line endings
            line.pop_back();
        size_t colon = line.find(':');
        if (colon == std::string::npos)
            continue;

        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        size_t      start = value.find_first_not_of(" \t");   // trim leading space
        value = (start == std::string::npos) ? "" : value.substr(start);

        if (key == "Status")                // e.g. "Status: 404 Not Found"
            res.status = std::atoi(value.c_str());
        else if (key == "Content-Type")
            res.contentType = value;
        else
            res.headers[key] = value;
    }
}

void Webserv::finishCgi(int cgiOutFd, Client& client, int errorCode)
{
    close(cgiOutFd);
    _cgiFdToClientOut.erase(cgiOutFd);
	client._cgiFdOut = -1;

    if (errorCode != 0)                        // CGI failed: replace output with an error page
        selectServer(client.getListenFd(), getRequestHost(client))
            ->serveErrorPage(client._response, errorCode);

    client._writeBuf = client.serializeResponse();
    client._bytesSent = 0;
    client._busy = true; // A response is now in flight; cleared by connectOut once it's fully sent.
    epollCtl(EPOLL_CTL_MOD, client._clientFd, EPOLLIN | EPOLLOUT);   // connectOut sends it, EPOLLIN keeps buffering any pipelined bytes
}

// void Webserv::cgiDone(int clientFd)
// {
// 	epollCtl(EPOLL_CTL_MOD, clientFd, EPOLLOUT);
// }

// Extracts the request's Host, lowercased and stripped of any :port suffix,
// for matching against server_name. Returns "" when no Host is present
// (e.g. a request that failed to parse), which makes selectServer fall back to the default server.
std::string	Webserv::getRequestHost(const Client& client)
{
	std::map<std::string, std::string>::const_iterator	it = client._request.headers.find("host");
	if (it == client._request.headers.end())
		return ("");

	std::string	host = it->second;
	size_t		colon = host.find(':');
	if (colon != std::string::npos)
		host = host.substr(0, colon);
	std::transform(host.begin(), host.end(), host.begin(), ::tolower);
	return (host);
}

void	Webserv::handleCGI(Client& client)
{
	client.setTimeCgi();
	int pipe_in[2] {-1,-1};
	int pipe_out[2];
	if (client._request.method == "POST")
		pipe(pipe_in); //check for failure
	pipe(pipe_out); //check for failure
	
	pid_t pid = fork();
	
	//child
	if (pid == 0)
	{
		Server *s = selectServer(client._listenFd, getRequestHost(client));
		const Location &l = s->getServerConfig().getLocation(client._request.uri);
		const std::string &scriptpath = l.indexPath;
		std::string servername = s->getServerConfig().getServerName();
		std::string serverport = _listenFdToPort[client._listenFd];
		Cgi cgi(l, client._request, scriptpath, servername, serverport);
		
		//check all for failure
		if (pipe_in[0] != -1)
		{
			dup2(pipe_in[0], STDIN_FILENO);
			close(pipe_in[0]);
			close(pipe_in[1]);
		}
		
		close(pipe_out[0]);
		close(pipe_out[1]);
		dup2(pipe_out[1], STDOUT_FILENO);

		execve(scriptpath.c_str(), cgi.getArgv(), cgi.getEnvp());
		exit(1);
		
	}

	//fork failure
	else if (pid == -1)
	{
		//call server failure function to send response to client, maybe throw
		if (pipe_in[0] != -1)
		{
			close(pipe_in[0]);
			close(pipe_in[1]);
		}
		close(pipe_out[0]);
		close(pipe_out[1]);
		serveError(client, 500, false);
	}

	//parent
	else
	{
		//check for failure
		if (pipe_in[0] != -1)
		{
			close(pipe_in[0]);
			setNonBlocking(pipe_in[1]);
		}
		close(pipe_out[1]);
		setNonBlocking(pipe_out[0]);
		_cgiFdToClientOut[pipe_out[0]] = client._clientFd;
		if (client._request.method == "POST")
		{
			_cgiFdToClientIn[pipe_in[1]] = client._clientFd;
			//add in to epoll
			epollCtl(EPOLL_CTL_ADD, pipe_in[1], EPOLLOUT);
		}
		epollCtl(EPOLL_CTL_ADD, pipe_out[0], EPOLLIN);
		_cgiPid[pid] = client._clientFd;
	}
}

// Among the servers behind listenFd, returns the one whose server_name matches host
// (case-insensitive). Falls back to the first server on that socket,
// which is the default server nginx would use when no server_name matches.
Server*	Webserv::selectServer(int listenFd, const std::string& host)
{
	const std::vector<Server*>&	candidates = _listenFdToServers.at(listenFd);

	for (Server* candidate : candidates)
	{
		std::string	name = candidate->getServerConfig().getServerName();
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);
		if (name == host)
			return (candidate);
	}
	return (candidates.front());
}

void Webserv::checkHealth()
{
	closeAndCleanCgi();

	auto	it = _clients.begin();

	while (it != _clients.end()) // Since the iterator was incremented within the loop, I changed this to a while loop.
	{
		std::cout << "DEBUG in checkHealth: Timecheck\n" << std::endl;
		if (it->second._cgiFdOut != -1 && it->second.checkTimeCgi() == -1)
		{
			auto it2 = _cgiPid.begin();
			while (it2 != _cgiPid.end())
			{
				if (it->first == it2->second)
				{
					kill(it2->first, SIGKILL);
					break;
				}
			}
			closeCgiPipes(it->first);
			serveError(it->second, 504, false);
		}
		if (it->second.checkTime() == -1)
		{
			int fd = it->first;
			it++;
			if (it->second._alive == false && it->second._bytesSent == 0)
				serveError(it->second, 408, true);
			else if (it->second._alive == true && it->second._buf.empty() == false && it->second._bytesSent == 0)
				serveError(it->second, 408, true);
			else
				closeAndRemoveFdFromClientList(fd);
			std::cout << "Connection timed out after 15 seconds of inactivity" << std::endl;
			// break ; // The client is closed and the iterator becomes invalid, so we have to break out.
		}
		else
			it++;
	}
}

ParseStatus	Webserv::parse(int clientFd)
{
	HttpParser	parser;

	return (parser.initParser(_clients.at(clientFd)));
}