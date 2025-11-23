#pragma once

#include "Configuration.hpp"

bool	validateHttpDirective(const Directive* node);
bool	validateServerDirective(const Directive* node);
bool	validateLocationDirective(const Directive* node);
bool	validateListenDirective(const Directive* node);
bool	validateRootDirective(const Directive* node);
bool	validateIndexDirective(const Directive* node);
bool	validateAutoIndexDirective(const Directive* node);
bool	validateErrorPageDirective(const Directive* node);
// bool	validateFastcgiPassDirective(const Directive* node);
// bool	validateFastcgiParamDirective(const Directive* node);
// bool	validateFastcgiIndexDirective(const Directive* node);
bool	validateReturnDirective(const Directive* node);
bool	validateAllowMethodsDirective(const Directive* node);
bool	validateClientMaxBodySizeDirective(const Directive* node);
bool	validateAllowOrDenyDirective(const Directive* node);	// Can be used to block certain IP Addresses from accessing a page.

std::pair<std::string, std::string>	parseAddressAndPort(const std::string& address);
bool 								isByte(std::string &number);
bool								validateAddress(const std::string& address);
bool								validatePort(const std::string& port);
bool								validateContext(const Directive* node);
bool								validateRequiredChildren(const Directive* node);

struct DirectiveDefinition
{
	std::string								name;
	bool									isBlock;
	size_t									minArgs;
	size_t									maxArgs;
	std::set<std::string>					validContexts;
	std::set<std::string>					requiredChildren;

	bool (*validateArgs)(const Directive*);
};


extern const std::map<std::string, DirectiveDefinition> NGINX_DIRECTIVE_SPECS;

class HttpStatus {
	public:
		static const int OK = 200;
		static const int CREATED = 201;
		static const int ACCEPTED = 202;
		static const int NO_CONTENT = 204;
		static const int MOVED_PERMANENTLY = 301;
		static const int FOUND = 302;
		static const int NOT_MODIFIED = 304;
		static const int BAD_REQUEST = 400;
		static const int UNAUTHORIZED = 401;
		static const int FORBIDDEN = 403;
		static const int NOT_FOUND = 404;
		static const int METHOD_NOT_ALLOWED = 405;
		static const int INTERNAL_SERVER_ERROR = 500;
		static const int NOT_IMPLEMENTED = 501;
		static const int BAD_GATEWAY = 502;
		static const int SERVICE_UNAVAILABLE = 503;

		static bool			isSuccess(int code) { return code >= 200 && code < 300; }
		static bool			isRedirect(int code) { return code >= 300 && code < 400; }
		static bool			isClientError(int code) { return code >= 400 && code < 500; }
		static bool			isServerError(int code) { return code >= 500 && code < 600; }
		static const char*	toString(int code);
};

static const char* toString(int code)
{
	switch(code)
	{
		case 200: return "200 OK";
		case 201: return "201 Created";
		case 202: return "202 Accepted";
		case 204: return "204 No Content";
		case 301: return "301 Moved Permanently";
		case 302: return "302 Found";
		case 304: return "304 Not Modified";
		case 400: return "400 Bad Request";
		case 401: return "401 Unauthorized";
		case 403: return "403 Forbidden";
		case 404: return "404 Not Found";
		case 405: return "405 Method Not Allowed";
		case 500: return "500 Internal Server Error";
		case 501: return "501 Not Implemented";
		case 502: return "502 Bad Gateway";
		case 503: return "503 Service Unavailable";
		default: return "Unknown Status";
	}
}
