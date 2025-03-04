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
    bool allowItems;

    void initializeDefaultProps();

public:
    Component();
    Component(boost::uuids::uuid id,
              std::chrono::system_clock::time_point createdOn,
              std::chrono::system_clock::time_point updatedOn);
    explicit Component(ComponentType type);
    Component(ComponentType type,
              boost::uuids::uuid id,
              std::chrono::system_clock::time_point createdOn,
              std::chrono::system_clock::time_point updatedOn);
    Component(ComponentType type, const std::map<std::string, std::string> &props, bool allowItems);

    // From BaseNode
    void generateCode(inja::Environment &env) const override;
    void updateFromJson(const nlohmann::json &json) override;
    std::shared_ptr<BaseNode> clone() const override;

    ComponentType getType() const;
    const std::map<std::string, std::string> &getProps() const;
    bool isAllowingItems() const;

    void setType(ComponentType type);
    void setProps(const std::map<std::string, std::string> &props);
    void setAllowItems(bool allow);
};

#endif // COMPONENT_H
