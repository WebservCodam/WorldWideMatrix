/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/03 15:42:28 by rkaras        #+#    #+#                 */
/*   Updated: 2025/10/24 15:34:05 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

// #include "HttpParser.hpp"
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <unistd.h>
// #include <vector>
// #include <iostream>
// #include <cstring>

// int main() {
//     const int port = 8080;
//     int listenFd = socket(AF_INET, SOCK_STREAM, 0);
//     if (listenFd < 0) { perror("socket"); return 1; }

//     sockaddr_in addr{};
//     addr.sin_family = AF_INET;
//     addr.sin_addr.s_addr = INADDR_ANY;
//     addr.sin_port = htons(port);

//     int opt = 1;
//     setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

//     if (bind(listenFd, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); return 1; }
//     if (listen(listenFd, 1) < 0) { perror("listen"); return 1; }

//     std::cout << "Listening on port " << port << "...\n";

//     int clientFd = accept(listenFd, nullptr, nullptr);
//     if (clientFd < 0) { perror("accept"); return 1; }

//     std::vector<char> buffer(8192);
//     ssize_t bytes = read(clientFd, buffer.data(), buffer.size());
//     if (bytes < 0) { perror("read"); return 1; }

//     std::string data(buffer.data(), bytes);

//     HttpParser parser;
//     ConnectionContext ctx;
//     parser.appendData(ctx, data.c_str(), data.size());

//     try {
//         ParseStatus status = parser.parseRequest(ctx);

//         if (status == ParseStatus::COMPLETE) {
//             std::cout << "Request parsed successfully!\n";
//             std::cout << "Method: " << ctx.request.method << "\n";
//             std::cout << "URI: " << ctx.request.uri << "\n";
//             std::cout << "Version: " << ctx.request.version << "\n";
//             std::cout << "Headers:\n";
//             for (auto &h : ctx.request.headers)
//                 std::cout << "  " << h.first << ": " << h.second << "\n";

//             if (!ctx.request.body.empty())
//                 std::cout << "Body: " << ctx.request.body << "\n";
//         } else if (status == ParseStatus::INCOMPLETE) {
//             std::cout << "Parsing incomplete – more data needed.\n";
//         } else {
//             std::cout << "Parsing error.\n";
//         }
//     } catch (const std::exception &e) {
//         std::cerr << "Exception: " << e.what() << "\n";
//     }

//     close(clientFd);
//     close(listenFd);
//     return 0;
// }


#include "HttpParser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv <request_file>\n";
		return 1;
	}

	std::ifstream infile(argv[1], std::ios::binary);
	if (!infile.is_open())
	{
		std::cerr << "Failed to open file: " << argv[1] << "\n";
		return 1;
	}

	std::ostringstream ss;
	ss << infile.rdbuf();
	std::string data = ss.str();

	HttpParser parser;
	ConnectionContext ctx;
	parser.appendData(ctx, data.c_str(), data.size());

	try
	{
		ParseStatus status = parser.parseRequest(ctx);

		if (status == ParseStatus::COMPLETE)
		{
			std::cout << "✅ Request parsed successfully!\n";
			std::cout << "Method: " << ctx.request.method << "\n";
			std::cout << "URI: " << ctx.request.uri << "\n";
			std::cout << "Version: " << ctx.request.version << "\n";
			std::cout << "Headers:\n";
			for (std::map<std::string, std::string>::const_iterator it = ctx.request.headers.begin();
				 it != ctx.request.headers.end(); ++it)
				std::cout << "  " << it->first << ": " << it->second << "\n";

			if (ctx.request.body.empty())
				std::cout << "(no body)\n";
			else
				std::cout << "Body: " << ctx.request.body << "\n";
		}
		else if (status == ParseStatus::INCOMPLETE)
			std::cout << "⚠️ Parsing incomplete – more data needed.\n";
		else
			std::cout << "❌ Parsing failed.\n";
	}
	catch (const std::exception &e)
	{
		std::cerr << "Exception during parsing: " << e.what() << "\n";
	}
}
