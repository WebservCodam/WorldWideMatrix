#pragma once

#include <iostream>
#include <utility>
#include <string>

std::pair<std::string, std::string>	parseAddressAndPort(const std::string& address);
bool								validateAddress(const std::string& address);
bool								isByte(std::string &number);
bool								validatePort(const std::string& port);
