/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpException.cpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/02/17 15:59:44 by rkaras        #+#    #+#                 */
/*   Updated: 2026/02/17 16:01:38 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HttpException.hpp"

HttpException::HttpException(int status, const std::string &message) : _status(status), _message(message) {}

int HttpException::getStatus() const
{
	return _status;
}

