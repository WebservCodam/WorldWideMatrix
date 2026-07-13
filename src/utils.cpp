#include "utils.hpp"

void	setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
	{
		throw std::runtime_error("fcntl error. Failure to set socket to non-blocking.");
	}
	if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1)
	{
		throw std::runtime_error("fcntl error. Failure to set socket to close on execute.");
	}
}
