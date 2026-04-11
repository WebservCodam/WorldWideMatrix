/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: vknape <vknape@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/09/15 13:53:26 by vknape        #+#    #+#                 */
/*   Updated: 2026/04/11 15:14:08 by lprieri       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

int setNonBlocking(int fd)
{
	return (fcntl(fd, F_SETFL, O_NONBLOCK));
}
