/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpException.hpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/02/17 15:55:14 by rkaras        #+#    #+#                 */
/*   Updated: 2026/02/17 16:02:22 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPEXCEPTION_H
#define HTTPEXCEPTION_H

#pragma once
#include <exception>
#include <string>


class HttpException
{
	private:
		int			_status;
		std::string	_message;
	
		public:
		HttpException(int status, const std::string &message);
		
		int getStatus() const;

		const char * what() const throw()
		{
			return _message.c_str();
		}
};

#endif /* !HTTPEXCEPTION_H */