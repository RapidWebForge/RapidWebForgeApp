#ifndef COMPONENT_H
#define COMPONENT_H

#include "../../models/component-type/componenttype.h"
#include <map>
#include <string>
#include <vector>

class Component
{
private:
    ComponentType type;
    std::map<std::string, std::string> props;
    std::vector<Component> nestedComponents;
    bool allowItems;

public:
    Component();
    Component(ComponentType type);
    Component(ComponentType type, const std::map<std::string, std::string> &props, bool allowItems);

    ComponentType getType() const;
    const std::map<std::string, std::string> &getProps() const;
    std::map<std::string, std::string> &getProps();
    const std::vector<Component> &getNestedComponents() const;
    std::vector<Component> &getNestedComponents();
    bool isAllowingItems() const;

    void setType(ComponentType type);
    void setProps(const std::map<std::string, std::string> &props);
    void addNestedComponent(const Component &component);
    void setNestedComponents(const std::vector<Component> &components);
    void initializeDefaultProps();
    void setAllowItems(bool allow);
};

#endif // COMPONENT_H
