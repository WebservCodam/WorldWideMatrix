/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils2.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/31 12:44:41 by rkaras        #+#    #+#                 */
/*   Updated: 2026/02/26 17:00:19 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"

bool	HttpParser::readLine(const char *buf, size_t length, size_t &pos, std::string &out)
{
	out.clear();

	while (pos < length)
	{
		char c = buf[pos++];

		if (c == '\r')
		{
			if (pos < length && buf[pos] == '\n')
			{
				++pos;
				return (true);
			}
			throw HttpException(400, "Malformed line: CR not followed by LF");
		}
		out += c;
	}
	return (!out.empty());
}