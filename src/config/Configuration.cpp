#include "Configuration.hpp"

ConfigFile::ConfigFile(std::vector<std::unique_ptr<Directive>> directives,
				const std::string& filename = "") : _directives(directives), _filename(filename) {}

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
