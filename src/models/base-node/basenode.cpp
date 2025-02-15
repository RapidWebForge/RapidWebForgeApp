#include "basenode.h"

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

// Child

void BaseNode::addChild(const std::shared_ptr<BaseNode> &child)
{
    children.push_back(child);
    child->setParent(shared_from_this());
}

void BaseNode::insertChild(int index, const std::shared_ptr<BaseNode> &child)
{
    if (index < 0 || index > children.size()) {
        throw std::out_of_range("Index out of bounds");
    }
    children.insert(children.begin() + index, child);
    child->setParent(shared_from_this());
}

void BaseNode::removeChild(int index)
{
    if (index < 0 || index >= children.size()) {
        throw std::out_of_range("Index out of bounds");
    }
    children[index]->setParent(nullptr);
    children.erase(children.begin() + index);
}

void BaseNode::clearChildren()
{
    for (auto &child : children) {
        child->setParent(nullptr);
    }
    children.clear();
}

const std::vector<std::shared_ptr<BaseNode>> &BaseNode::getChildren() const
{
    return children;
}

// Parent

void BaseNode::setParent(const std::shared_ptr<BaseNode> &parentNode)
{
    parent = parentNode;
}

std::shared_ptr<BaseNode> BaseNode::getParent() const
{
    return parent.lock();
}
