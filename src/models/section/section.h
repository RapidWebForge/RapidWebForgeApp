#ifndef SECTION_H
#define SECTION_H

#include "../component/component.h"
#include <string>
#include <vector>

class Section
{
private:
    std::string name;
    std::vector<Component> components;

public:
    Section();
    Section(const std::string &name);
    Section(const std::string &name, const std::vector<Component> &components);

    std::string getName() const;
    const std::vector<Component> &getComponents() const;
    std::vector<Component> &getComponents();

    void setName(const std::string &name);
    void setComponents(const std::vector<Component> &components);
};

#endif // SECTION_H
