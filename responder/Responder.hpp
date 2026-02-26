/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Responder.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/31 15:37:05 by rkaras        #+#    #+#                 */
/*   Updated: 2026/02/24 17:20:00 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONDER_H
#define RESPONDER_H

#pragma once

#include <string>
#include <sstream>
#include <fstream>
// #include "../httpparser/HttpParser.hpp"

class HttpRequest;

class Responder
{
	private:
		std::string handleGet(const HttpRequest &req, bool keepAlive);
		std::string statusText(int status);
		
	public:
		std::string buildResponse(const HttpRequest &req, bool keepAlive);
		std::string buildErrorResponse(int status, bool keepAlive);
		
};

#endif /* !RESPONDER_H */