#include "genericnode.h"

GenericNode::GenericNode()
    : BaseNode("GenericNode", std::chrono::system_clock::now(), std::chrono::system_clock::now())
{}

GenericNode::GenericNode(const std::string &nodeType)
    : BaseNode(nodeType, std::chrono::system_clock::now(), std::chrono::system_clock::now())
{
    generateUniqueId();
}

void GenericNode::generateCode(inja::Environment &env) const
{
    // No-op: Root node does not generate code
}

void GenericNode::updateFromJson(const nlohmann::json &json)
{
    // No-op: Root node does not update from JSON
}

std::shared_ptr<BaseNode> GenericNode::clone() const
{
    auto cloned = std::make_shared<GenericNode>(*this); // Copia los datos base
    // Ahora, reemplaza el vector de hijos con copias profundas de cada hijo.
    cloned->children.clear();
    for (const auto &child : this->children) {
        cloned->children.push_back(child->clone());
    }
    return cloned;
}

bool GenericNode::isDifferentFrom(const std::shared_ptr<BaseNode> &other) const
{
    return true;
}
