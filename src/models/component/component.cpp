#include "component.h"

Component::Component()
    : type("")
    , props()
{}

Component::Component(const std::string &type)
    : type(type)
    , props()
{}

Component::Component(const std::string &type, const std::map<std::string, std::string> &props)
    : type(type)
    , props(props)
{}

// Getters

std::string Component::getType() const
{
    return type;
}

const std::map<std::string, std::string> &Component::getProps() const
{
    return props;
}

std::map<std::string, std::string> &Component::getProps()
{
    return props;
}

// Setters

void Component::setType(const std::string &type)
{
    this->type = type;
}

void Component::setProps(const std::map<std::string, std::string> &props)
{
    this->props = props;
}
