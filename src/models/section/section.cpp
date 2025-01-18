#include "section.h"

Section::Section()
    : name("")
    , components()
{}

Section::Section(const std::string &name)
    : name(name)
    , components()
{}

Section::Section(const std::string &name, const std::vector<Component> &components)
    : name(name)
    , components(components)
{}

// Getters

std::string Section::getName() const
{
    return name;
}

const std::vector<Component> &Section::getComponents() const
{
    return components;
}

std::vector<Component> &Section::getComponents()
{
    return components;
}

// Setters

void Section::setName(const std::string &name)
{
    this->name = name;
}
void Section::setComponents(const std::vector<Component> &components)
{
    this->components = components;
}
