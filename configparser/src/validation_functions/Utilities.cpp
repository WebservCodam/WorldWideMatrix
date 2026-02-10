#include "../../include/Configuration.hpp"


std::pair<std::string, std::string>	parseAddressAndPort(const std::string& address)
{
	std::pair<std::string, std::string>	addressAndPort = {"", ""};
	std::string	addressPart;
	std::string	portPart;
	size_t		pos;

	if (address.empty())
		return (addressAndPort);
	pos = address.find(":");
	if (pos == address.npos)
		return (addressAndPort);
	else
	{
		addressPart = address.substr(0, pos);
		portPart = address.substr(pos + 1, address.size() - (pos + 1));
		addressAndPort = {std::move(addressPart), std::move(portPart)};
	}

	std::cout << "DEBUG in parserAddressAndPort - Address: " << addressAndPort.first << " Port: " << addressAndPort.second << std::endl;

	return (addressAndPort);
}

bool	validateAddress(const std::string& address)
{
	size_t		currentPos = 0;
	size_t		nextPos;
	size_t		iterations = 0;
	std::string	currentChunk;

	if (address.empty())
		return (false);

	if (address == "localhost")
		return (true);

	while (currentPos < address.length())
	{
		nextPos = address.find(".", currentPos);
		if (nextPos == std::string::npos)
			currentChunk = address.substr(currentPos);
		else
			currentChunk = address.substr(currentPos, nextPos - currentPos);

		if (!isByte(currentChunk))
			return (false);

		iterations++;
		if (nextPos == std::string::npos)
			break;
		currentPos = nextPos + 1;
	}
	if (iterations != 4)
		return (false);
	return (true);
}

bool	validatePort(const std::string& port)
{
	if (port.empty())
		return (false);
	try
	{
		int portNumber = std::stoi(port);
		if (port.size() != std::to_string(portNumber).length())
			throw (std::runtime_error("Wrong port"));
		if (portNumber >= 1 && portNumber <= 65535)
			return (true);
	}
	catch(const std::exception& e)
	{
		return (false);
	}
	return (false);
}

bool isByte(std::string &number)
{
	if (number.empty())
		return (false);

    try
	{
		int	num = std::stoi(number);
		if (number.size() != std::to_string(num).length())
			return (false);
		if (num > 255 || num < 0)
			return (false);
	}
	catch(const std::exception& e)
	{
		return (false);
	}	
    return (true);
}
