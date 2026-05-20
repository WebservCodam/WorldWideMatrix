#include "../Configuration.hpp"

void	validateLocationDirective(Directive* node)
{
	std::string	root = getRoot(node);
	std::string	location = joinPath(root, node->getParameter(0));
	checkPath(location, ErrorType::VALIDATOR, "Location: " + location, true);

	return (validateBlockDirective(node));
}

/**
 * @brief
 * 
 */
Location	ConfigFile::processLocation(Directive* directive)
{
	Directive*		server = nullptr;
	Directive*		indexDirective;
	Location		location = Location();
	std::string		root = "";
	std::string		index = "";

	//	Inherit from server (THESE COULD BE OPTIMIZED BY HAVING THE MAIN FUNCTION DO THIS AND PASSING THE PRE-WORKED LOCATION WITH THESE VALUES, for every location)
	server = getServerDirective(directive);
	if (server == nullptr)
		throw ConfigError::semantics("Location directive is not in any server.", directive);
	root = getRoot(server);
	location.autoindex = getAutoindex(server);

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

		if (child->getParameters().empty()) // Would this ever be the case (in our server)?
			continue ;

		if (name == "root")
			root = child->getParameter(0);
		else if (name == "index")
			index = child->getParameter(0);
		else if (name == "autoindex")
		{
			std::string autoindexParam = child->getParameter(0);
			location.autoindex = (autoindexParam == "on" || autoindexParam == "true");
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

	return (location);
}
