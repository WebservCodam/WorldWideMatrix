#include "Configuration.hpp"

// ----- DIRECTIVE CLASS -----

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

// ----- GETTERS ------

const std::string&	Directive::getParameter(size_t i) const
{
	if (i < 0 || i >= _parameters.size())
		throw std::out_of_range("Parameter index out of range");

	return (_parameters.at(i));
}

Directive*	Directive::getChild(size_t i)
{
	if (i < 0 || i >= _children.size())
		return (nullptr);

	return (_children[i].get());
}

Directive*	Directive::getChild(const std::string& name)
{
	for (const std::unique_ptr<Directive>& child : _children)
	{
		if (child && child->getName() == name)
			return (child.get());
	}
	return (nullptr); // Throw error in the calling function.
}

std::vector<Directive*>	Directive::getChildren()
{
	std::vector<Directive*>	result;

	result.reserve(_children.size());
	for (const std::unique_ptr<Directive>& child : _children)
	{
		result.push_back(child.get());
	}
	return (result);
}
