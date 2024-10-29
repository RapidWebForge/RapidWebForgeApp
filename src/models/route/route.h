#ifndef ROUTE_H
#define ROUTE_H

#include <string>

class Route
{
private:
    std::string component;
    std::string path;

public:
    Route();
    Route(const std::string &component, const std::string &path);

    std::string getComponent() const;
    std::string getPath() const;

    void setComponent(const std::string &component);
    void setPath(const std::string &path);
};

#endif // ROUTE_H
