#ifndef BASENODE_H
#define BASENODE_H

#include <boost/uuid/uuid.hpp>
#include <chrono>
#include <inja/inja.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

class BaseNode : public std::enable_shared_from_this<BaseNode>
{
protected:
    std::string nodeType;
    std::vector<std::shared_ptr<BaseNode>> children;
    std::weak_ptr<BaseNode> parent;
    std::chrono::system_clock::time_point createdOn;
    std::chrono::system_clock::time_point updatedOn;
    boost::uuids::uuid id;

    void generateUniqueId(std::string type = "",
                          bool allowItems = false,
                          std::map<std::string, std::string> props = {});

public:
    BaseNode();
    explicit BaseNode(const std::string &nodeType);
    explicit BaseNode(const std::string &nodeType,
                      std::chrono::system_clock::time_point createdOn,
                      std::chrono::system_clock::time_point updatedOn);
    explicit BaseNode(const std::string &nodeType,
                      boost::uuids::uuid id,
                      std::chrono::system_clock::time_point createdOn,
                      std::chrono::system_clock::time_point updatedOn);
    virtual ~BaseNode() = default;

    virtual void generateCode(inja::Environment &env) const = 0;
    virtual void updateFromJson(const nlohmann::json &json) = 0;
    virtual std::shared_ptr<BaseNode> clone() const = 0;
    virtual bool isDifferentFrom(const std::shared_ptr<BaseNode> &other) const = 0;

    const std::string &getNodeType() const;
    std::chrono::system_clock::time_point getCreatedOn() const;
    std::chrono::system_clock::time_point getUpdatedOn() const;
    boost::uuids::uuid getId() const;

    void update();

    // Gestión de hijos
    void addChild(const std::shared_ptr<BaseNode> &child);
    void insertChild(int index, const std::shared_ptr<BaseNode> &child);
    void removeChild(int index);
    void removeChild(std::vector<std::shared_ptr<BaseNode>>::iterator it);
    void clearChildren();
    std::vector<std::shared_ptr<BaseNode>> &getChildren();
    const std::vector<std::shared_ptr<BaseNode>> &getChildren() const; // Versión const
    bool removeChildForById(std::string id);
    // Gestión del nodo padre
    void setParent(const std::shared_ptr<BaseNode> &parentNode);
    std::shared_ptr<BaseNode> getParent() const;
};

#endif // BASENODE_H
