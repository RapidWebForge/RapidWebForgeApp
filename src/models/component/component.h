#ifndef COMPONENT_H
#define COMPONENT_H

#include <map>
#include <string>

class Component
{
private:
    std::string type;
    std::map<std::string, std::string> props;

public:
    Component();
    Component(const std::string &type, const std::map<std::string, std::string> &props);

    std::string getType() const;
    const std::map<std::string, std::string> &getProps() const;
    std::map<std::string, std::string> &getProps();

    void setType(const std::string &type);
    void setProps(const std::map<std::string, std::string> &props);
};

#endif // COMPONENT_H
