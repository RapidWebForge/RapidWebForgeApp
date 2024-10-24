#ifndef COMPONENT_H
#define COMPONENT_H

#include "../../models/component-type/componenttype.h"
#include <map>
#include <string>

class Component
{
private:
    ComponentType type;
    std::map<std::string, std::string> props;

public:
    Component();
    Component(ComponentType type);
    Component(ComponentType type, const std::map<std::string, std::string> &props);

    ComponentType getType() const;
    const std::map<std::string, std::string> &getProps() const;
    std::map<std::string, std::string> &getProps();

    void setType(ComponentType type);
    void setProps(const std::map<std::string, std::string> &props);
    void initializeDefaultProps();
};

#endif // COMPONENT_H
