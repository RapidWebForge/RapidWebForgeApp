#include "projectmanager.h"
#include <fmt/core.h>

ProjectManager::ProjectManager()
{
    createTable();
}

void ProjectManager::createTable()
{
    // SQL statements for creating tables
    const char *sqlDatabases = "CREATE TABLE IF NOT EXISTS databases ("
                               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                               "server TEXT NOT NULL,"
                               "port TEXT NOT NULL,"
                               "user TEXT NOT NULL,"
                               "password TEXT NOT NULL,"
                               "database_name TEXT NOT NULL);";

    const char *sqlProjects = "CREATE TABLE IF NOT EXISTS projects ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                              "name TEXT NOT NULL,"
                              "description TEXT NOT NULL,"
                              "path TEXT NOT NULL,"
                              "created_at TEXT NOT NULL,"
                              "updated_at TEXT NOT NULL,"
                              "database_id INTEGER,"
                              "FOREIGN KEY (database_id) REFERENCES databases(id));";

    char *errorMessage = nullptr;
    // Using singleton
    sqlite3 *db = Database::getInstance().getConnection();

    // Execute the second statement to create the databases table
    if (sqlite3_exec(db, sqlDatabases, 0, 0, &errorMessage) != SQLITE_OK) {
        fmt::print(stderr, "Error creating table 'databases': {}\n", errorMessage);
        sqlite3_free(errorMessage);
        return; // Early return if there's an error
    } else {
        fmt::print("Table 'databases' created or already exists.\n");
    }

    // Execute the first statement to create the projects table
    if (sqlite3_exec(db, sqlProjects, 0, 0, &errorMessage) != SQLITE_OK) {
        fmt::print(stderr, "Error creating table 'projects': {}\n", errorMessage);
        sqlite3_free(errorMessage);
        return; // Early return if there's an error
    } else {
        fmt::print("Table 'projects' created or already exists.\n");
    }
}

void ProjectManager::createProject(const Project &project)
{
    sqlite3_stmt *stmt;
    sqlite3 *db = Database::getInstance().getConnection();

    std::string sqlDatabase = "INSERT INTO databases (server, port, user, password, database_name) "
                              "VALUES (?, ?, ?, ?, ?);";

    DatabaseData dbData(project.getDatabaseData());

    if (sqlite3_prepare_v2(db, sqlDatabase.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, dbData.getServer().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, dbData.getPort().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, dbData.getUser().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, dbData.getPassword().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, dbData.getDatabaseName().c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fmt::print(stderr, "Error inserting database: {}\n", sqlite3_errmsg(db));
        }
    } else {
        fmt::print(stderr, "Error preparing the declaration: {}\n", sqlite3_errmsg(db));
    }

    // Get the id of the last inserted database
    int databaseId = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);

    std::string sqlProject = "INSERT INTO projects (name, description, path, created_at, "
                             "updated_at, database_id) VALUES (?, ?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sqlProject.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, project.getName().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, project.getDescription().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, project.getPath().c_str(), -1, SQLITE_STATIC);
        // Convert `createdAt` and `updatedAt` to string
        std::string createdAtStr = project.getCreatedAt();
        std::string updatedAtStr = project.getUpdatedAt();
        sqlite3_bind_text(stmt, 4, createdAtStr.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, updatedAtStr.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 6, databaseId);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fmt::print(stderr, "Error inserting project: {}\n", sqlite3_errmsg(db));
        }
    } else {
        fmt::print(stderr, "Error preparing the declaration: {}\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

std::optional<DatabaseData> ProjectManager::getDatabaseById(int databaseId)
{
    sqlite3 *db = Database::getInstance().getConnection();
    sqlite3_stmt *stmt;

    // Get by ID query
    std::string sql = "SELECT * FROM databases WHERE id = ?;";

    DatabaseData dbData;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id, databaseId);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                // Extract DatabaseData details
                std::string server = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
                std::string port = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
                std::string user = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
                std::string password = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
                std::string databaseName = reinterpret_cast<const char *>(
                    sqlite3_column_text(stmt, 4));

                dbData = DatabaseData(server, port, user, password, databaseName);
            } else {
                fmt::print(stderr, "Database with ID: {} was not found\n", databaseId);
                sqlite3_finalize(stmt);
                return std::nullopt; // No database found for the given `database_id`
            }
        } else {
            fmt::print(stderr,
                       "Error preparing the statement for database: {}\n",
                       sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return std::nullopt; // Error in preparing the statement        }
        }
        sqlite3_finalize(stmt);
        return std::nullopt;
    }
}

std::optional<Project> ProjectManager::getProjectById(int projectId)
{
    sqlite3 *db = Database::getInstance().getConnection();
    sqlite3_stmt *stmt;

    // Get by ID query
    std::string sql = "SELECT * FROM projects WHERE id = ?;";

    Database dbData;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int projectId = sqlite3_column_int(stmt, 0);
            std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            std::string description = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            std::string path = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
            int databaseId = sqlite3_column_int(stmt, 4);

            dbData = getDatabaseById(databaseId);

            Project project(projectId, name, description, path, dbData);

            sqlite3_finalize(stmt);
            return project;
        } else {
            fmt::print(stderr, "Project with ID: {} was not found\n", id);
        }
    } else {
        fmt::print(stderr, "Error preparing the declaration: {}\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<Project> ProjectManager::getAllProjects()
{
    sqlite3 *db = Database::getInstance().getConnection();
    std::vector<Project> projects;
    // Get all query
    std::string sql = "SELECT * FROM projects;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int projectId = sqlite3_column_int(stmt, 0);
            std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            std::string description = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            Project project(projectId, name, description);
            projects.push_back(project);
        }
    } else {
        fmt::print(stderr, "Error preparing the declaration: {}\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
    return projects;
}

void ProjectManager::updateProject(const Project &project)
{
    sqlite3 *db = Database::getInstance().getConnection();
    // Update query
    std::string sql = "UPDATE projects SET name = ?, description = ?, updated_at = ? WHERE id = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, project.getName().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, project.getDescription().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, project.getUpdatedAt().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, project.getId());

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fmt::print(stderr, "Error updating project: {}\n", sqlite3_errmsg(db));
        }
    } else {
        fmt::print(stderr, "Error preparing the declaration: {}\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

void ProjectManager::deleteProjectById(int id)
{
    sqlite3 *db = Database::getInstance().getConnection();
    std::string sql = "DELETE FROM projects WHERE id = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fmt::print(stderr, "Error removing project: {}\n", sqlite3_errmsg(db));
        } else {
            fmt::print("Project with ID {} remove succesfully.\n", id);
        }
    } else {
        fmt::print(stderr, "Error preparing the declaration: {}\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}
