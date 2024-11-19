#include "component.h"

Component::Component()
    : type(ComponentType::Undefined)
    , nestedComponents()
{
    initializeDefaultProps();
}

Component::Component(ComponentType type)
    : type(type)
    , nestedComponents()
{
    initializeDefaultProps();
}

Component::Component(ComponentType type,
                     const std::map<std::string, std::string> &props,
                     bool allowItems)
    : type(type)
    , props(props)
    , nestedComponents()
    , allowItems(allowItems)
{
    initializeDefaultProps();
}

void Component::initializeDefaultProps()
{
    auto it = componentPropertiesMap.find(type);
    if (it != componentPropertiesMap.end()) {
        props = it->second;
    }

    switch (type) {
    case ComponentType::Form:
    case ComponentType::HorizontalLayout:
    case ComponentType::VerticalLayout:
    case ComponentType::ModelLayout:
        this->allowItems = true;
        break;
    default:
        this->allowItems = false;
        break;
    }
}

void Component::addNestedComponent(const Component &component)
{
    nestedComponents.push_back(component);
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

const std::vector<Component> &Component::getNestedComponents() const
{
    return this->nestedComponents;
}

std::vector<Component> &Component::getNestedComponents()
{
    return this->nestedComponents;
}

bool Component::isAllowingItems() const
{
    return this->allowItems;
}

// Setters

void Component::setType(ComponentType type)
{
    this->type = type;

    initializeDefaultProps();
}

void Component::setProps(const std::map<std::string, std::string> &props)
{
    this->props = props;
}

void Component::setAllowItems(bool allow)
{
    this->allowItems = allow;
}

void Component::setNestedComponents(const std::vector<Component> &components)
{
    this->nestedComponents = components;
}
