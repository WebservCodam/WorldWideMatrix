#include "Webserv.hpp"
#include "Client.hpp"

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
	addListeningSocketToEpoll(listenFd);
	return (listenFd);
}

void	Webserv::addListeningSocketToEpoll(int listenFd)
{
	static struct epoll_event	event;

	event.data.fd = listenFd;
	event.events = EPOLLIN | EPOLLPRI | EPOLLHUP;
	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, listenFd, &event) == -1)
		throw std::runtime_error("Server add to epoll failed");
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
			int	eventFd = events[i].data.fd;

			std::cout << "DEBUG: We're in the startServers for loop." << std::endl;
			if (_listenFdToServers.find(eventFd) != _listenFdToServers.end())
			{
				connectNew(eventFd);
				connections++;
			}
			else if (_clients.find(eventFd) != _clients.end())
			{
				if (events[i].events & EPOLLIN)
					connectIn(eventFd);
				else if (events[i].events & EPOLLOUT)
					connectOut(eventFd);
			}
			else
				throw std::runtime_error("This FD doesn't belong to a server nor a client.");
		}
		checkHealth();
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

		struct epoll_event	event;

		event.events = EPOLLIN; // We want to be notified when the fd is ready for reading. Removed the edge-triggered event, because it might be impossible to implement with the errno constraint.
		event.data.fd = clientFd;
		if (epoll_ctl(_epfd, EPOLL_CTL_ADD, clientFd, &event) == -1)
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

void Webserv::connectIn(int clientFd)
{
	char		buffer[8192] = {0};
	ssize_t		count = 0; // Bytes read.
	ParseStatus	status;

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
	
	// std::cout << "DEBUG in connectIn: PRINTING BUFFER" << std::endl;
	// std::cout << buffer << std::endl;

	status = parse(clientFd); // Branch the status received into conditionals.

	if (status == INCOMPLETE)
	{
		std::cout << "Parsing incomplete; returning so we connectIn again." << std::endl;
		return ; // So it goes back to connectIn on next loop. And since it's level-triggered, the event will still be there.
	}
	else if (status == ERROR)
	{
		// The parser set the error status; build its body and serve it as is instead of routing the request (which would overwrite the status).
		selectServer(client.getListenFd(), getRequestHost(client))->serveErrorPage(client._response, client._response.status);
	}
	else // COMPLETE: route the request and build the response now, while we wait for EPOLLOUT. connectOut only pushes bytes.
		selectServer(client.getListenFd(), getRequestHost(client))->handleRequest(client);

	client._writeBuf = client.serializeResponse();
	client._bytesSent = 0;

	struct epoll_event	event;

	event.events = EPOLLOUT;
	event.data.fd = clientFd;
	if (epoll_ctl(_epfd, EPOLL_CTL_MOD, clientFd, &event) == -1) // When this function goes out of scope, what happens to event? It gets transfered? Does epoll_ctl copy it?
	{
		perror("Epoll_ctl: switch to EPOLLOUT failed");
		closeAndRemoveFdFromClientList(clientFd);
	}
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

	// Response fully sent: re-arm for the next request on this connection.
	client._writeBuf.clear();
	client._bytesSent = 0;
	client._response = HttpResponse(); // Headers like Allow or Location must not leak into the next response.

	if (client._alive == false)
	{
		closeAndRemoveFdFromClientList(clientFd);
		return ;
	}

	struct epoll_event	event;
	event.events = EPOLLIN;
	event.data.fd = clientFd;
	if (epoll_ctl(_epfd, EPOLL_CTL_MOD, clientFd, &event) == -1)
	{
		perror("Epoll_ctl: switch to EPOLLIN failed");
		closeAndRemoveFdFromClientList(clientFd);
	}
}

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
	auto	it = _clients.begin();

	while (it != _clients.end()) // Since the iterator was incremented within the loop, I changed this to a while loop.
	{
		std::cout << "DEBUG in checkHealth: Timecheck\n" << std::endl;
		if (it->second.checkTime() == -1)
		{
			int fd = it->first;
			closeAndRemoveFdFromClientList(it->first);
			std::cout << "Connection timed out after 15 seconds of inactivity" << std::endl;
			break ; // The client is closed and the iterator becomes invalid, so we have to break out.
		}
		it++;
	}
}

ParseStatus	Webserv::parse(int clientFd)
{
	HttpParser	parser;

	return (parser.initParser(_clients.at(clientFd)));
}
