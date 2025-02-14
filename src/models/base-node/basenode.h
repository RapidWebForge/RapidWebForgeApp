#ifndef BASENODE_H
#define BASENODE_H

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

public:
    BaseNode();
    explicit BaseNode(const std::string &nodeType);
    virtual ~BaseNode() = default;

    virtual void generateCode(inja::Environment &env) const = 0;
    virtual void updateFromJson(const nlohmann::json &json) = 0;

    const std::string &getNodeType() const;

    // Gestión de hijos
    void addChild(const std::shared_ptr<BaseNode> &child);
    const std::vector<std::shared_ptr<BaseNode>> &getChildren() const;
    // Gestión del nodo padre
    void setParent(const std::shared_ptr<BaseNode> &parentNode);
    std::shared_ptr<BaseNode> getParent() const;
};

#endif // BASENODE_H
