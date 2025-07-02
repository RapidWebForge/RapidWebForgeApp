#ifndef NODEOPERATION_H
#define NODEOPERATION_H

#include "../base-node/basenode.h"

enum class OperationType { Insert, Modify, Delete };

struct NodeOperation
{
    OperationType type;
    std::shared_ptr<BaseNode> node;

    // Constructor
    NodeOperation(OperationType opType, std::shared_ptr<BaseNode> &nodePtr)
        : type(opType)
        , node(nodePtr)
    {}
};

#endif // NODEOPERATION_H
