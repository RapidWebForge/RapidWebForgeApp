#include "view.h"

View::View()
    : name("")
    , components()
{}

View::View(const std::string &name)
    : name(name)
    , components()
{}

View::View(const std::string &name, const std::vector<Component> &components)
    : name(name)
    , components(components)
{}

// Getters

std::string View::getName() const
{
    return name;
}

const std::vector<Component> &View::getComponents() const
{
    return components;
}

std::vector<Component> &View::getComponents()
{
    return components;
}

// Setters

void View::setName(const std::string &name)
{
    this->name = name;
}
void View::setComponents(const std::vector<Component> &components)
{
    this->components = components;
}
