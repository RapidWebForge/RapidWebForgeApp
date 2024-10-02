#ifndef PROJECT_MANAGER_H
#define PROJECT_MANAGER_H

#include "../../models/project/project.h"
#include "../../utils/database/database.h"
#include <optional>
#include <sqlite3.h>
#include <string>
#include <vector>

class ProjectManager
{
public:
    ProjectManager();
    ~ProjectManager() = default;

    void createProject(const Project &project);
    std::optional<Project> getProjectById(int id);
    std::vector<Project> getAllProjects();
    void updateProject(const Project &project);
    void deleteProjectById(int id);

private:
    sqlite3 *db;
    void createTable();
};

#endif // PROJECT_MANAGER_H
