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

#include "HttpParser.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <cstring>

int main() {
    const int port = 8080;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); return 1; }
    if (listen(server_fd, 1) < 0) { perror("listen"); return 1; }

    std::cout << "Listening on port " << port << "...\n";

    int client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd < 0) { perror("accept"); return 1; }

    std::vector<char> buffer(8192);
    ssize_t bytes = read(client_fd, buffer.data(), buffer.size());
    if (bytes < 0) { perror("read"); return 1; }

    std::string data(buffer.data(), bytes);

    HttpParser parser;
    ConnectionContext ctx;
    parser.appendData(ctx, data.c_str(), data.size());

    try {
        ParseStatus status = parser.parseRequest(ctx);

        if (status == ParseStatus::COMPLETE) {
            std::cout << "Request parsed successfully!\n";
            std::cout << "Method: " << ctx.request.method << "\n";
            std::cout << "URI: " << ctx.request.uri << "\n";
            std::cout << "Version: " << ctx.request.version << "\n";
            std::cout << "Headers:\n";
            for (auto &h : ctx.request.headers)
                std::cout << "  " << h.first << ": " << h.second << "\n";

            if (!ctx.request.body.empty())
                std::cout << "Body: " << ctx.request.body << "\n";
        } else if (status == ParseStatus::INCOMPLETE) {
            std::cout << "Parsing incomplete – more data needed.\n";
        } else {
            std::cout << "Parsing error.\n";
        }
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
