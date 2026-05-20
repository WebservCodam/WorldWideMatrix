#pragma once

#include <string>

// Plain config value types shared between Configuration.hpp and ServerConfig.hpp.
// Kept in their own header so neither of those two has to include the other,
// which breaks the circular include between them.

struct ListenDirective
{
	std::string	address;
	std::string	port;

	ListenDirective(const std::string& addr, const std::string& p) : address(addr), port(p) {}
};

struct	ErrorPage
{
	int			errorCode;
	bool		isRedirect = false;
	int			redirectCode = -1;
	std::string	URI;

	ErrorPage() = default;
	ErrorPage(int code, const std::string& uri) : errorCode(code), isRedirect(false), redirectCode(-1), URI(uri) {}
};

struct	ReturnPage
{
	int			code = -1;
	bool		isURI = false;
	std::string	page = "";	// Which can store a path to a page if it is URI or it can simply store the whole page here.
};

struct	Location
{
	std::string		name = "";			// This represents the location we're trying to access.
	std::string		dirPath = "";		// This is the name with root prepended.
	std::string		indexPath = "";		// This is the full path that goes to the index. root + location + (index?).
	std::string		cgiParam = "";
	bool			isCGI = false;
	ReturnPage		returnPage = ReturnPage();
	bool			autoindex = false;
	bool			getMethod = false;
	bool			postMethod = false;
	bool			deleteMethod = false;
};
