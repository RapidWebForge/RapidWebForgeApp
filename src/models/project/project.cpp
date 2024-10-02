#include "project.h"
#include <iomanip>
#include <iostream>
#include <sstream>

// Constructor con parámetros
Project::Project(int id, const std::string &name, const std::string &description)
    : id(id)
    , name(name)
    , description(description)
    , createdAt(std::chrono::system_clock::now())
    , updatedAt(std::chrono::system_clock::now())
{
    createdAt = std::chrono::system_clock::now();
    updatedAt = createdAt;
}

// Constructor vacío
Project::Project()
    : id(0)
    , name("")
    , description("")
    , createdAt(std::chrono::system_clock::now())
    , updatedAt(std::chrono::system_clock::now())
{
    createdAt = std::chrono::system_clock::now();
    updatedAt = createdAt;
}

// Getters
int Project::getId() const
{
    return id;
}

std::string Project::getName() const
{
    return name;
}

std::string Project::getDescription() const
{
    return description;
}

std::string Project::getCreatedAt() const
{
    std::time_t createdAtTimeT = std::chrono::system_clock::to_time_t(createdAt);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&createdAtTimeT), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string Project::getUpdatedAt() const
{
    std::time_t updatedAtTimeT = std::chrono::system_clock::to_time_t(updatedAt);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&updatedAtTimeT), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
// Setters
void Project::setName(const std::string &newName)
{
    name = newName;
    setUpdatedAt();
}

void Project::setDescription(const std::string &newDescription)
{
    description = newDescription;
    setUpdatedAt();
}

void Project::setUpdatedAt()
{
    updatedAt = std::chrono::system_clock::now();
}
