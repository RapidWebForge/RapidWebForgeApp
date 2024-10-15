#ifndef VIEW_H
#define VIEW_H

#include "../component/component.h"
#include <string>
#include <vector>

class View
{
private:
    std::string name;
    std::vector<Component> components;

public:
    View();
    View(const std::string &name);
    View(const std::string &name, const std::vector<Component> &components);

    std::string getName() const;
    const std::vector<Component> &getComponents() const;
    std::vector<Component> &getComponents();

    void setName(const std::string &name);
    void setComponents(const std::vector<Component> &components);
};

#endif // VIEW_H
