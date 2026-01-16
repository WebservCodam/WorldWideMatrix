#pragma once

// #include "Configuration.hpp"

struct ListenDirective
{
	std::string	address;	// defaults to "0.0.0.0"
	std::string	port;		// defaults to 80
    
	ListenDirective(const std::string& addr = "0.0.0.0", std::string p = "80") : address(addr), port(p) {}
};

class	Location
{
	private:
		std::string					_path;
		std::string					_root;
		std::string					_index;
		bool						_autoindex;
		bool						_getMethod;
		bool						_postMethod;
		bool						_deleteMethod;

	public:
		Location() = delete;
		Location(const std::string& path, const std::string& root = "", const std::string& index = "", bool autoindex = false, bool getMethod = true, bool postMethod = false, bool deleteMethod = false);
		~Location() = default;

		const std::string&	getPath() const;
		const std::string&	getRoot() const;
		const std::string&	getIndex() const;
		bool				getAutoindex() const;
		bool				getGetMethod() const;
		bool				getPostMethod() const;
		bool				getDeleteMethod() const;


};

class	ServerConfig
{
	private:
		std::string						_serverName;
		std::vector<ListenDirective>	_listenDirectives;
		size_t							_maxBodySize;
		std::map<int, std::string>		_errors;	//	The idea is that different error codes can return the same error. But this might overcomplicate things.
		std::vector<Location>			_locations;

	public:
		ServerConfig() = delete;
		ServerConfig(const std::string& serverName, const std::vector<ListenDirective>	listenDirectives, size_t maxBodySize, const std::map<int, std::string>& errors, const std::vector<Location>& locations);
		~ServerConfig() = default;

		const std::string&						getServerName() const;
		const std::vector<ListenDirective>&		getListenDirectives() const;	// Create function to get a port from an address and viceversa
		size_t									getMaxBodySize() const;
		const std::map<int, std::string>&		getErrors() const;
		const std::vector<Location>&			getLocations() const;


};
