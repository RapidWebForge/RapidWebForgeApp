#include "route.h"

Route::Route()
    : component("")
    , path("")
{}

Route::Route(const std::string &component, const std::string &path)
    : component(component)
    , path(path)
{}

// Getters

std::string Route::getComponent() const
{
    return component;
}

std::string Route::getPath() const
{
    return path;
}

// Setters

void Route::setComponent(const std::string &component)
{
    this->component = component;
}

void Route::setPath(const std::string &path)
{
    this->path = path;
}
