/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: rkaras <rkaras@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/10/03 15:42:28 by rkaras        #+#    #+#                 */
/*   Updated: 2025/10/03 15:48:40 by rkaras        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./test_parser <request_file>\n";
        return 1;
    }

    std::ifstream infile(argv[1]);
    if (!infile.good())
    {
        std::cerr << "Failed to open file: " << argv[1] << "\n";
        return 1;
    }

    // Prepare parser and connection context
    HttpParser parser;
    ConnectionContext ctx;

    // Read the entire file content and feed to the parser
    std::string line;
    while (std::getline(infile, line))
    {
        // getline strips the newline, so re-append "\n" or "\r\n"
        line.append("\r\n");
        parser.appendData(ctx, line.c_str(), line.size());
    }

    // Attempt to parse
    try
    {
        ParseStatus status = parser.parseRequest(ctx);
        if (status == ParseStatus::COMPLETE)
        {
            std::cout << "Request parsed successfully!\n";
            std::cout << "Method: "   << ctx.request.method  << "\n";
            std::cout << "URI: "      << ctx.request.uri     << "\n";
            std::cout << "Version: "  << ctx.request.version << "\n";

            std::cout << "Headers:\n";
            for (std::map<std::string, std::string>::const_iterator it = ctx.request.headers.begin();
                 it != ctx.request.headers.end(); ++it)
            {
                std::cout << "  " << it->first << ": " << it->second << "\n";
            }

            if (!ctx.request.body.empty())
            {
                std::cout << "Body:\n" << ctx.request.body << "\n";
            }
            else
            {
                std::cout << "No body or empty body.\n";
            }
        }
        else if (status == ParseStatus::INCOMPLETE)
        {
            std::cout << "Parsing incomplete – more data needed.\n";
        }
        else
        {
            std::cout << "Parsing error.\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception during parsing: " << e.what() << "\n";
    }

    return 0;
}