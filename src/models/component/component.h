#ifndef COMPONENT_H
#define COMPONENT_H

#include "../base-node/basenode.h"
#include "../component-type/componenttype.h"
#include <boost/uuid/uuid.hpp>
#include <chrono>
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
    std::chrono::system_clock::time_point createdOn;
    std::chrono::system_clock::time_point updatedOn;
    boost::uuids::uuid id;

    void initializeDefaultProps();
    void generateUniqueId();

public:
    Component();
    Component(boost::uuids::uuid id,
              std::chrono::system_clock::time_point createdOn,
              std::chrono::system_clock::time_point updatedOn);
    Component(ComponentType type);
    Component(ComponentType type,
              boost::uuids::uuid id,
              std::chrono::system_clock::time_point createdOn,
              std::chrono::system_clock::time_point updatedOn);
    Component(ComponentType type, const std::map<std::string, std::string> &props, bool allowItems);

    ComponentType getType() const;
    const std::map<std::string, std::string> &getProps() const;
    const std::vector<std::shared_ptr<BaseNode>> &getNestedComponents() const;
    bool isAllowingItems() const;
    std::chrono::system_clock::time_point getCreatedOn() const;
    std::chrono::system_clock::time_point getUpdatedOn() const;
    boost::uuids::uuid getId() const;

    void update();
    void addNestedComponent(const std::shared_ptr<BaseNode> &component);
    void insertNestedComponent(int index, const std::shared_ptr<BaseNode> &component);

    void setType(ComponentType type);
    void setProps(const std::map<std::string, std::string> &props);
    void setNestedComponents(const std::vector<std::shared_ptr<BaseNode>> &components);
    void setAllowItems(bool allow);
};

#endif // COMPONENT_H
