#include "genericnode.h"

GenericNode::GenericNode()
    : BaseNode("GenericNode")
{}

GenericNode::GenericNode(const std::string &nodeType)
    : BaseNode(nodeType)
{}

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
    return std::make_shared<GenericNode>(*this);
}
