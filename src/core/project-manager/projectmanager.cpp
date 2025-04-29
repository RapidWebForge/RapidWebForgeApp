#include "projectmanager.h"
#include <filesystem>
#include <fmt/core.h>

ProjectManager::ProjectManager()
{
    createTable();
}

void ProjectManager::createTable()
{
    createDatabasesTable();
    createProjectsTable();
}

namespace fs = std::filesystem;

void ProjectManager::createDatabasesTable()
{
    sqlite3 *db = Database::getInstance().getConnection();
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS databases (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            server TEXT NOT NULL,
            port TEXT NOT NULL,
            user TEXT NOT NULL,
            password TEXT NOT NULL,
            database_name TEXT NOT NULL
        );
    )";
    executeSQL(db, sql, nullptr);
}

void ProjectManager::createProjectsTable()
{
    sqlite3 *db = Database::getInstance().getConnection();
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS projects (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            description TEXT,
            path TEXT NOT NULL,
            frontendPort TEXT NOT NULL,
            backendPort TEXT NOT NULL,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            database_id INTEGER,
            versions BOOLEAN DEFAULT 0,  -- Nueva columna para versiones
            FOREIGN KEY(database_id) REFERENCES databases(id)
        );
    )";
    executeSQL(db, sql, nullptr);
}

std::optional<DatabaseData> ProjectManager::getDatabaseById(int databaseId)
{
    sqlite3 *db = Database::getInstance().getConnection();
    std::optional<DatabaseData> dbDataOpt;

    std::string sql
        = "SELECT id, server, port, user, password, database_name FROM databases WHERE id = ?;";
    executeSQL(
        db,
        sql,
        [&](sqlite3_stmt *stmt) { sqlite3_bind_int(stmt, 1, databaseId); },
        [&](sqlite3_stmt *stmt) {
            int id = sqlite3_column_int(stmt, 0);
            std::string server = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            std::string port = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            std::string user = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
            std::string password = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
            std::string databaseName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));

            dbDataOpt = DatabaseData(id, server, port, user, password, databaseName);
        });

    return dbDataOpt;
}

bool ProjectManager::executeSQL(sqlite3 *db,
                                const std::string &sql,
                                const std::function<void(sqlite3_stmt *)> &bindParams,
                                const std::function<void(sqlite3_stmt *)> &processResults)
{
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (bindParams) {
            bindParams(stmt);
        }
        if (processResults) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                processResults(stmt);
            }
        } else {
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                fmt::print(stderr, "Error executing SQL: {}\n", sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return false;
            }
        }
    } else {
        fmt::print(stderr, "Error preparing SQL: {}\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

void ProjectManager::createProject(const Project &project)
{
    // Create folder
    std::string pathToCreate = project.getPath();

    // Check if the directory exists
    try {
        if (fs::create_directories(pathToCreate)) {
            fmt::print("Directorio creado con éxito en: {}\n", pathToCreate);
        } else {
            fmt::print("El directorio ya existe o no se pudo crear.\n");
            return;
        }
    } catch (const fs::filesystem_error &e) {
        fmt::print(stderr, "Error al crear el directorio: {}", e.what());
        return;
    }

    // Añadir la nueva base de datos del project a sqlite
    sqlite3 *db = Database::getInstance().getConnection();
    DatabaseData dbData = project.getDatabaseData();

    std::string sqlDatabase = "INSERT INTO databases (server, port, user, password, database_name) "
                              "VALUES (?, ?, ?, ?, ?);";

    std::string server = dbData.getServer();
    std::string port = dbData.getPort();
    std::string user = dbData.getUser();
    std::string password = dbData.getPassword();
    std::string dbName = dbData.getDatabaseName();

    executeSQL(db, sqlDatabase, [&](sqlite3_stmt *stmt) {
        sqlite3_bind_text(stmt, 1, server.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, port.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, user.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, password.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, dbName.c_str(), -1, SQLITE_STATIC);
    });

    int databaseId = sqlite3_last_insert_rowid(db);

    // Add the new project's info to the sqlite database
    std::string sqlProject
        = "INSERT INTO projects (name, description, path, frontendPort, backendPort, created_at, "
          "updated_at, database_id, versions) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";

    // Evitar punteros perdidos
    std::string projectName = project.getName();
    std::string description = project.getDescription();
    std::string path = project.getPath();
    std::string frontendPort = project.getFrontendPort();
    std::string backendPort = project.getBackendPort();
    std::string createdAt = project.getCreatedAt();
    std::string updatedAt = project.getUpdatedAt();
    bool versions = project.getVersions();

    executeSQL(db, sqlProject, [&](sqlite3_stmt *stmt) {
        sqlite3_bind_text(stmt, 1, projectName.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, path.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, frontendPort.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, backendPort.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 6, createdAt.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 7, updatedAt.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 8, databaseId);
        sqlite3_bind_int(stmt, 9, versions ? 1 : 0);
    });
}

std::optional<Project> ProjectManager::getProjectById(int projectId)
{
    sqlite3 *db = Database::getInstance().getConnection();
    std::optional<Project> projectOpt;

    std::string sql = "SELECT id, name, description, path, frontendPort, backendPort, database_id, "
                      "created_at, updated_at, versions FROM projects WHERE id = ?;";
    executeSQL(
        db,
        sql,
        [&](sqlite3_stmt *stmt) { sqlite3_bind_int(stmt, 1, projectId); },
        [&](sqlite3_stmt *stmt) {
            int id = sqlite3_column_int(stmt, 0);
            std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            std::string description = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            std::string path = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
            std::string frontendPort = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
            std::string backendPort = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
            int databaseId = sqlite3_column_int(stmt, 6);
            bool versions = sqlite3_column_int(stmt, 9) == 1;

            auto dbDataOpt = getDatabaseById(databaseId);
            if (dbDataOpt) {
                projectOpt = Project(id,
                                     name,
                                     description,
                                     path,
                                     dbDataOpt.value(),
                                     frontendPort,
                                     backendPort,
                                     versions);
            }
        });

    return projectOpt;
}

std::optional<Project> ProjectManager::getProjectByName(std::string name)
{
    sqlite3 *db = Database::getInstance().getConnection();
    std::optional<Project> projectOpt;

    std::string sql = "SELECT id, name, description, path, frontendPort, backendPort, database_id, "
                      "created_at, updated_at, versions FROM projects WHERE name = ?;";
    executeSQL(
        db,
        sql,
        [&](sqlite3_stmt *stmt) { sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT); },
        [&](sqlite3_stmt *stmt) {
            int id = sqlite3_column_int(stmt, 0);
            std::string pName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            std::string description = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            std::string path = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
            std::string frontendPort = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
            std::string backendPort = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
            int databaseId = sqlite3_column_int(stmt, 6);
            bool versions = sqlite3_column_int(stmt, 9) == 1;

            auto dbDataOpt = getDatabaseById(databaseId);
            if (dbDataOpt) {
                projectOpt = Project(id,
                                     pName,
                                     description,
                                     path,
                                     dbDataOpt.value(),
                                     frontendPort,
                                     backendPort,
                                     versions);
            }
        });

    return projectOpt;
}

std::vector<Project> ProjectManager::getAllProjects()
{
    sqlite3 *db = Database::getInstance().getConnection();
    std::vector<Project> projects;

    std::string sql = "SELECT id, name, description, path, frontendPort, backendPort, database_id, "
                      "created_at, updated_at, versions FROM projects;";
    executeSQL(db, sql, nullptr, [&](sqlite3_stmt *stmt) {
        int projectId = sqlite3_column_int(stmt, 0);
        std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        std::string description = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        std::string path = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        std::string frontendPort = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        std::string backendPort = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
        int databaseId = sqlite3_column_int(stmt, 6);
        bool versions = sqlite3_column_int(stmt, 9) == 1;

        auto dbDataOpt = getDatabaseById(databaseId);
        if (dbDataOpt) {
            projects.push_back(Project(projectId,
                                       name,
                                       description,
                                       path,
                                       dbDataOpt.value(),
                                       frontendPort,
                                       backendPort,
                                       versions));
        }
    });

    return projects;
}

void ProjectManager::updateProject(const Project &project)
{
    sqlite3 *db = Database::getInstance().getConnection();

    std::string sqlUpdateProject = "UPDATE projects SET name = ?, description = ?, "
                                   "updated_at = ? WHERE id = ?;";

    // Evitar punteros perdidos
    std::string projectName = project.getName();
    std::string description = project.getDescription();
    std::string updatedAt = project.getUpdatedAt();
    int projectId = project.getId();

    executeSQL(db, sqlUpdateProject, [&](sqlite3_stmt *stmt) {
        sqlite3_bind_text(stmt, 1, projectName.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, updatedAt.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, projectId);
    });
}

void ProjectManager::deleteProjectById(int id)
{
    sqlite3 *db = Database::getInstance().getConnection();
    int databaseId = -1;

    std::string sqlGetDatabaseId = "SELECT database_id FROM projects WHERE id = ?;";
    executeSQL(
        db,
        sqlGetDatabaseId,
        [&](sqlite3_stmt *stmt) { sqlite3_bind_int(stmt, 1, id); },
        [&](sqlite3_stmt *stmt) { databaseId = sqlite3_column_int(stmt, 0); });

    if (databaseId == -1) {
        fmt::print(stderr, "No project found with ID: {}\n", id);
        return;
    }

    std::string sqlDeleteProject = "DELETE FROM projects WHERE id = ?;";
    executeSQL(db, sqlDeleteProject, [&](sqlite3_stmt *stmt) { sqlite3_bind_int(stmt, 1, id); });

    std::string sqlDeleteDatabase = "DELETE FROM databases WHERE id = ?;";
    executeSQL(db, sqlDeleteDatabase, [&](sqlite3_stmt *stmt) {
        sqlite3_bind_int(stmt, 1, databaseId);
    });
}

bool ProjectManager::isProjectAvailable(const std::string &projectName)
{
    sqlite3 *db = Database::getInstance().getConnection();
    bool exists = false;

    std::string sql = "SELECT COUNT(*) FROM projects WHERE name = ?;";

    executeSQL(
        db,
        sql,
        [&](sqlite3_stmt *stmt) {
            sqlite3_bind_text(stmt, 1, projectName.c_str(), -1, SQLITE_STATIC);
        },
        [&](sqlite3_stmt *stmt) {
            int count = sqlite3_column_int(stmt, 0);
            exists = (count > 0);
        });

    return !exists;
}

bool ProjectManager::isDatabaseAvailable(const std::string &databaseName)
{
    sqlite3 *db = Database::getInstance().getConnection();
    bool exists = false;

    std::string sql = "SELECT COUNT(*) FROM databases WHERE database_name = ?;";

    executeSQL(
        db,
        sql,
        [&](sqlite3_stmt *stmt) {
            sqlite3_bind_text(stmt, 1, databaseName.c_str(), -1, SQLITE_STATIC);
        },
        [&](sqlite3_stmt *stmt) {
            int count = sqlite3_column_int(stmt, 0);
            exists = (count > 0);
        });

    return !exists;
}
