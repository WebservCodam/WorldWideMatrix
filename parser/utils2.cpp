/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/31 12:44:41 by rkaras        #+#    #+#                 */
/*   Updated: 2025/10/31 15:14:53 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"

// void	HttpParser::appendData(ConnectionContext &ctx, const char *data, size_t len)
// {
// 	ctx.buffer.append(data, len);
// }

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
			throw std::runtime_error("Malformed line: CR not followed by LF");
		}
		out += c;
	}
	return (!out.empty());
}