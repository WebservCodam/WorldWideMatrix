#include "../../include/Configuration.hpp"

//	----- LOCATION -----

Location::Location(const std::string& path, const std::string& root, const std::string& index, bool autoindex, bool getMethod, bool postMethod, bool deleteMethod)
	: _path(path), _root(root), _index(index), _autoindex(autoindex), _getMethod(getMethod), _postMethod(postMethod), _deleteMethod(deleteMethod)
{
}

const std::string&	Location::getPath() const
{
	return (this->_path);
}

const std::string&	Location::getRoot() const
{
	return (this->_root);
}

const std::string&	Location::getIndex() const
{
	return (this->_index);
}

bool	Location::getAutoindex() const
{
	return (this->_autoindex);
}

bool	Location::getGetMethod() const
{
	return (this->_getMethod);
}

bool	Location::getPostMethod() const
{
	return (this->_postMethod);
}

bool	Location::getDeleteMethod() const
{
	return (this->_deleteMethod);
}

//	----- SERVER -----

Server::Server(const std::string& serverName,
				const std::map<std::string, std::string>& addressesAndPorts,
				size_t maxBodySize,
				const std::map<int, std::string>& errors,
				const std::vector<Location>& locations)
	: _serverName(serverName), _addressesAndPorts(addressesAndPorts), _maxBodySize(maxBodySize), _errors(errors), _locations(locations)
{
}

const std::string&	Server::getServerName() const
{
	return (this->_serverName);
}

const std::map<std::string, std::string>&	Server::getAddressesAndPorts() const
{
	return (this->_addressesAndPorts);
}

size_t	Server::getMaxBodySize() const
{
	return (this->_maxBodySize);
}

const std::map<int, std::string>&	Server::getErrors() const
{
	return (this->_errors);
}

const std::vector<Location>&	Server::getLocations() const
{
	return (this->_locations);
}
