#ifndef COMPONENT_H
#define COMPONENT_H

#include "../base-node/basenode.h"
#include "../component-type/componenttype.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

class Component : public BaseNode
{
private:
    ComponentType type;
    std::map<std::string, std::string> props;
    std::vector<std::shared_ptr<BaseNode>> nestedComponents;
    bool allowItems;

public:
    Component();
    Component(ComponentType type);
    Component(ComponentType type, const std::map<std::string, std::string> &props, bool allowItems);

    void initializeDefaultProps();
    void addNestedComponent(const std::shared_ptr<BaseNode> &component);

    ComponentType getType() const;
    const std::map<std::string, std::string> &getProps() const;
    const std::vector<std::shared_ptr<BaseNode>> &getNestedComponents() const;
    bool isAllowingItems() const;

    void setType(ComponentType type);
    void setProps(const std::map<std::string, std::string> &props);
    void setNestedComponents(const std::vector<std::shared_ptr<BaseNode>> &components);
    void setAllowItems(bool allow);
};

#endif // COMPONENT_H
