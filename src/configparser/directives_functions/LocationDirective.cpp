#include "../Configuration.hpp"

void	validateLocationDirective(Directive* node)
{
	std::string			root = getRoot(node);
	const std::string&	locationPath = node->getParameter(0);
	std::string			location = joinPath(root, locationPath);
	checkPath(location, ErrorType::VALIDATOR, "Location: " + location, true);

	return (validateBlockDirective(node));
}

Location	ConfigFile::processLocation(Directive* directive)
{
	Directive*		server = nullptr;
	Directive*		indexDirective;
	Location		location = Location();
	std::string		root = "";
	std::string		index = "";

	server = getServerDirective(directive);
	if (server == nullptr)
		throw ConfigError::semantics("Location directive is not in any server.", directive);
	root = getRoot(server);
	location.autoindex = getAutoindex(server);
	location.maxBodySize = processClientMaxBodySize(server->getChild("client_max_body_size"));

	Directive*	serverCgi = server->getChild("cgi_handler");
	if (serverCgi)
	{
		location.cgiExtension = serverCgi->getParameter(0);
		location.cgiInterpreter = serverCgi->getParameter(1);
	}

	indexDirective = server->getChild("index");
	if (indexDirective)
		index = indexDirective->getParameter(0);
	else
		index = "index.html";

	location.name = trimPathName(directive->getParameter(0));

	std::vector<Directive*> locationChildren = directive->getChildren();
	for (Directive* child : locationChildren)
	{
		const std::string& name = child->getName();

		if (name == "return")
		{
			location.returnPage = processReturnPage(child);
			continue ;
		}

		if (child->getParameters().empty())
			continue ;

		if (name == "root")
			root = child->getParameter(0);
		else if (name == "client_max_body_size")
			location.maxBodySize = processClientMaxBodySize(child);
		else if (name == "index")
			index = child->getParameter(0);
		else if (name == "autoindex")
		{
			std::string autoindexParam = child->getParameter(0);
			location.autoindex = (autoindexParam == "on" || autoindexParam == "true");
		}
		else if (name == "upload_path")
			location.uploadPath = joinPath(".", child->getParameter(0));
		else if (name == "cgi_handler")
		{
			location.cgiExtension = child->getParameter(0);
			location.cgiInterpreter = child->getParameter(1);
		}
		else if (name == "methods")
		{
			for (const std::string& method : child->getParameters())
			{
				if (method == "GET")
					location.getMethod = true;
				else if (method == "POST")
					location.postMethod = true;
				else if (method == "DELETE")
					location.deleteMethod = true;
			}
			if (location.getMethod == false && location.postMethod == false && location.deleteMethod == false)
			{
				location.getMethod = true;
				location.postMethod = true;
				location.deleteMethod = true;
			}
		}
	}

	//	Build full path
	location.dirPath = joinPath(root, location.name);
	if (!index.empty())
		location.indexPath = joinPath(location.dirPath, index);

	bool	isCgi = !location.cgiExtension.empty() || !location.cgiInterpreter.empty();
	if (location.postMethod && location.uploadPath.empty() && !isCgi)
		throw ConfigError::semantics("Location '" + location.name + "' allows POST but has no upload_path directive.", directive);

	return (location);
}
