#include "component.h"

Component::Component()
    : type(ComponentType::Undefined)
    , props()
{
    initializeDefaultProps();
}

Component::Component(ComponentType type)
    : type(type)
    , props()
{
    initializeDefaultProps();
}

Component::Component(ComponentType type, const std::map<std::string, std::string> &props)
    : type(type)
    , props(props)
{
    initializeDefaultProps();
}

void Component::initializeDefaultProps()
{
    auto it = componentPropertiesMap.find(type);
    if (it != componentPropertiesMap.end()) {
        props = it->second;
    }
}

// Getters

ComponentType Component::getType() const
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

void Component::setType(ComponentType type)
{
    this->type = type;
}

void Component::setProps(const std::map<std::string, std::string> &props)
{
    this->props = props;
}
