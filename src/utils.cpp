/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vknape <vknape@student.codam.nl>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 13:53:26 by vknape            #+#    #+#             */
/*   Updated: 2026/07/10 11:02:06 by vknape           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
