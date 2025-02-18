#ifndef GENERICNODE_H
#define GENERICNODE_H

#include "../base-node/basenode.h"

class GenericNode : public BaseNode
{
public:
    GenericNode();
    GenericNode(const std::string &nodeType); // ✅ New constructor

    // From BaseNode
    void generateCode(inja::Environment &env) const override;
    void updateFromJson(const nlohmann::json &json) override;
    std::shared_ptr<BaseNode> clone() const override;
};

#endif // GENERICNODE_H
