/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Responder.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/31 15:37:05 by rkaras        #+#    #+#                 */
/*   Updated: 2025/10/31 15:42:30 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONDER_H
#define RESPONDER_H

#include "../http_parser/HttpParser.hpp"


class Responder
{
	private:
		std::string	getStatusText(int code) const;
		std::string	getDefaultErrorPage(int code) const;
		std::string	getContentType(const std::string &path) const;
		
		std::string	handleRequest(const HttpRequest &req) const;
		
	public:
		Responder() = default;
		Responder(const Responder& other) = delete;
		Responder&	operator=(const Responder& other) = delete;
		~Responder() = default;

		std::string respond(const HttpRequest &req, bool keepAlive = false);
};

#endif /* !RESPONDER_H */