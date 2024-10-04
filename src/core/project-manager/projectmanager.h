#ifndef PROJECT_MANAGER_H
#define PROJECT_MANAGER_H

#include "../../models/project/project.h"
#include "../../utils/database/database.h"
#include <functional>
#include <optional>
#include <sqlite3.h>
#include <vector>

class ProjectManager
{
public:
    ProjectManager();

    bool executeSQL(sqlite3 *db,
                    const std::string &sql,
                    const std::function<void(sqlite3_stmt *)> &bindParams,
                    const std::function<void(sqlite3_stmt *)> &processResults = nullptr);
    void createProject(const Project &project);
    std::optional<Project> getProjectById(int projectId);
    std::vector<Project> getAllProjects();
    void updateProject(const Project &project);
    void deleteProjectById(int id);

private:
    void createTable();
    void createDatabasesTable();
    void createProjectsTable();
    std::optional<DatabaseData> getDatabaseById(int databaseId);
};

#endif // PROJECT_MANAGER_H
