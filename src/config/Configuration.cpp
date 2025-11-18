#include "Configuration.hpp"

ConfigFile::ConfigFile(std::vector<std::unique_ptr<Directive>> directives) : _directives(directives) {}

const std::vector<std::unique_ptr<Directive>>&	ConfigFile::getDirectives() const
{
	return (this->_directives);
}

const Directive*	ConfigFile::findDirective(const std::string& name) const
{
	for (const std::unique_ptr<Directive>& directive : this->_directives)
	{
		if (directive->getName() == name)
			return (directive.get()); // Extracts raw pointer from unique_ptr
	}
	return (nullptr);
}

std::vector<const Directive*>	ConfigFile::findAllDirectives(const std::string& name) const
{
	std::vector<const Directive*>	result;

    for (const std::unique_ptr<Directive>& directive : this->_directives)
    {
        if (directive->getName() == name)
            result.push_back(directive.get());
    }
    return (result);
}

// --- DIRECTIVE CLASS ---

Directive::Directive(
    size_t line,
    size_t column,
    const std::string& name,
    const std::string& context,
    std::vector<std::string> parameters,
    std::vector<std::unique_ptr<Directive>> children)
    : _line(line),
      _column(column),
      _name(name),
      _context(context),
      _parameters(std::move(parameters)),
      _children(std::move(children))
{}

// Getters

size_t	Directive::getLine() const
{
	return (this->_line);
}

size_t	Directive::getColumn() const
{
	return (this->_column);
}

const std::string&	Directive::getName() const
{
	return (this->_name);
}

const std::string&	Directive::getContext() const
{
	return (this->_context);
}

const std::string&	Directive::getParameter(size_t i) const
{
	if (i < 0 || i >= _parameters.size())
		throw std::out_of_range("Parameter index out of range");

	return (_parameters.at(i));
}

const std::vector<std::string>&	Directive::getParameters() const
{
	return (this->_parameters);
}

const Directive*	Directive::getChild(size_t i) const
{
	if (i < 0 || i >= _children.size())
		return (nullptr); // Throw exception

	return (_children[i].get());
}

std::vector<const Directive*>	Directive::getChildren() const
{
	std::vector<const Directive*>	result;

	result.reserve(_children.size());
	for (const std::unique_ptr<Directive>& child : _children)
	{
		result.push_back(child.get());
	}
	return (result);
}

// Setters

void Directive::setLine(size_t line)
{
	this->_line = line;
}

void Directive::setColumn(size_t column)
{
	this->_column = column;
}
void Directive::setName(const std::string& name)
{
	this->_name = name;
}
void Directive::setContext(const std::string& context)
{
	this->_context = context;
}
void Directive::setParameters(const std::vector<std::string>& parameters)
{
	this->_parameters = parameters;
}

void Directive::addChild(std::unique_ptr<Directive> child)
{
	this->_children.push_back(std::move(child));
}
