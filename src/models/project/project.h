#ifndef PROJECT_H
#define PROJECT_H

#include "../database-data/databasedata.h"
#include <chrono>
#include <string>

class Project
{
private:
    int id;
    std::string name;
    std::string description;
    std::string path;
    DatabaseData databaseData;
    std::string frontendPort;
    std::string backendPort;
    std::chrono::system_clock::time_point createdAt;
    std::chrono::system_clock::time_point updatedAt;

public:
    // Constructors
    Project(const Project &project);
    Project(int id,
            const std::string &name,
            const std::string &description,
            const std::string &path,
            const DatabaseData &databaseData,
            const std::string &frontendPort,
            const std::string &backendPort);
    Project();

    // Getters
    int getId() const;
    std::string getName() const;
    std::string getDescription() const;
    std::string getPath() const;

    const DatabaseData &getDatabaseData() const;
    DatabaseData &getDatabaseData();

    std::string getFrontendPort() const;
    std::string getBackendPort() const;

    std::string getCreatedAt() const;
    std::chrono::system_clock::time_point getCreatedAtChrono() const;
    std::string getUpdatedAt() const;
    std::chrono::system_clock::time_point getUpdatedAtChrono() const;

    std::string timePointToString(const std::chrono::system_clock::time_point &tp) const;

    // Setters
    void setName(const std::string &newName);
    void setDescription(const std::string &newDescription);
    void setPath(const std::string &newPath);
    void setFrontendPort(const std::string &frontendPort);
    void setBackendPort(const std::string &backendPort);
    void setUpdatedAt();
};

#endif // PROJECT_H
