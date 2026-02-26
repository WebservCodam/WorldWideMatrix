/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Responder.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/31 15:36:59 by rkaras        #+#    #+#                 */
/*   Updated: 2026/02/24 17:21:09 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Responder.hpp"
#include "../httpparser/HttpParser.hpp"

std::string Responder::buildResponse(const HttpRequest &req, bool keepAlive)
{
	if (req.method == "GET")
		//return handleGet
	else if (req.method == "POST")
		//return handlePost
	else if (req.method == "DELETE")
		//return handleDelete

	
}
