#include "project.h"
#include <iomanip>
#include <sstream>

// Empty constructor
Project::Project()
    : id(0)
    , name("")
    , description("")
    , path("")
    , databaseData()
    , frontendPort("9000")
    , backendPort("3000")
    , createdAt(std::chrono::system_clock::now())
    , updatedAt(createdAt)
{}

// Constructor with parameters
Project::Project(int id,
                 const std::string &name,
                 const std::string &description,
                 const std::string &path,
                 const DatabaseData &databaseData,
                 const std::string &frontendPort,
                 const std::string &backendPort,
                 bool versions) // Añadir versions
    : id(id)
    , name(name)
    , description(description)
    , path(path)
    , databaseData(databaseData)
    , frontendPort(frontendPort)
    , backendPort(backendPort)
    , versions(versions) // Inicializar versions
    , createdAt(std::chrono::system_clock::now())
    , updatedAt(createdAt)
{}

// Constructor with Project
Project::Project(const Project &project)
    : id(project.getId())
    , name(project.getName())
    , description(project.getDescription())
    , path(project.getPath())
    , databaseData(project.getDatabaseData())
    , frontendPort(project.getFrontendPort())
    , backendPort(project.getBackendPort())
    , createdAt(project.getCreatedAtChrono())
    , updatedAt(project.getUpdatedAtChrono())
{}

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

std::string Project::getPath() const
{
    return path;
}

const DatabaseData &Project::getDatabaseData() const
{
    return databaseData;
}

DatabaseData &Project::getDatabaseData()
{
    return databaseData;
}

std::string Project::getFrontendPort() const
{
    return frontendPort;
}

std::string Project::getBackendPort() const
{
    return backendPort;
}

std::string Project::timePointToString(const std::chrono::system_clock::time_point &tp) const
{
    std::time_t timeT = std::chrono::system_clock::to_time_t(tp);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string Project::getCreatedAt() const
{
    return timePointToString(createdAt);
}

std::chrono::system_clock::time_point Project::getCreatedAtChrono() const
{
    return createdAt;
}

std::string Project::getUpdatedAt() const
{
    return timePointToString(updatedAt);
}

std::chrono::system_clock::time_point Project::getUpdatedAtChrono() const
{
    return updatedAt;
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

void Project::setPath(const std::string &newPath)
{
    path = newPath;
    setUpdatedAt();
}

void Project::setFrontendPort(const std::string &newFrontendPort)
{
    frontendPort = newFrontendPort;
    setUpdatedAt();
}

void Project::setBackendPort(const std::string &newBackendPort)
{
    backendPort = newBackendPort;
    setUpdatedAt();
}

void Project::setUpdatedAt()
{
    updatedAt = std::chrono::system_clock::now();
}

// Getter y Setter para versions
bool Project::getVersions() const
{
    return versions;
}

void Project::setVersions(bool newVersions)
{
    versions = newVersions;
    setUpdatedAt();
}
