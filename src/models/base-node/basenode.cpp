#include "BaseNode.h"

BaseNode::BaseNode()
    : nodeType("Unknown")
{}

BaseNode::BaseNode(const std::string &nodeType)
    : nodeType(nodeType)
{}

const std::string &BaseNode::getNodeType() const
{
    return nodeType;
}

void BaseNode::addChild(const std::shared_ptr<BaseNode> &child)
{
    children.push_back(child);
    child->setParent(shared_from_this());
}

const std::vector<std::shared_ptr<BaseNode>> &BaseNode::getChildren() const
{
    return children;
}

void BaseNode::setParent(const std::shared_ptr<BaseNode> &parentNode)
{
    parent = parentNode;
}

std::shared_ptr<BaseNode> BaseNode::getParent() const
{
    return parent.lock();
}
