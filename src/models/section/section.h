#ifndef SECTION_H
#define SECTION_H

#include "../base-node/basenode.h"
#include <memory>
#include <string>
#include <vector>

class Section : public BaseNode
{
private:
    std::string name;
    std::vector<std::shared_ptr<BaseNode>> components; // Nodos hijos (Section o Component)

public:
    Section();
    explicit Section(const std::string &name);
    Section(const std::string &name, const std::vector<std::shared_ptr<BaseNode>> &components);

    std::string getName() const;
    const std::vector<std::shared_ptr<BaseNode>> &getComponents() const;

    void setName(const std::string &name);
    void setComponents(const std::vector<std::shared_ptr<BaseNode>> &components);

    // Métodos útiles
    void addComponent(const std::shared_ptr<BaseNode> &component);
    void insertComponent(int index, const std::shared_ptr<BaseNode> &component);
    bool removeComponentByName(const std::string &name);
    void clearComponents();
};

#endif // SECTION_H
