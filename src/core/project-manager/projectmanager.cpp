#include "projectmanager.h"
#include <fmt/core.h>
#include <iomanip>
#include <iostream>
#include <sstream>

ProjectManager::ProjectManager()
{
    createTable();
}

void ProjectManager::createTable()
{
    // Creation query
    const char *sql = "CREATE TABLE IF NOT EXISTS projects ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "name TEXT NOT NULL,"
                      "description TEXT NOT NULL,"
                      "created_at TEXT NOT NULL,"
                      "updated_at TEXT NOT NULL);";

    char *errorMessage = nullptr;
    // Using singleton
    sqlite3 *db = Database::getInstance().getConnection();

    if (sqlite3_exec(db, sql, 0, 0, &errorMessage) != SQLITE_OK) {
        fmt::print(stderr, "Error creating table: {}\n", errorMessage);
        sqlite3_free(errorMessage);
    } else {
        fmt::print("Table 'projects' created or already exists.\n");
    }
}
void ProjectManager::createProject(const Project &project)
{
    sqlite3 *db = Database::getInstance().getConnection();
    // Insert query
    std::string sql
        = "INSERT INTO projects (name, description, created_at, updated_at) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, project.getName().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, project.getDescription().c_str(), -1, SQLITE_STATIC);
        // Convert `createdAt` and `updatedAt` to string
        std::string createdAtStr = project.getCreatedAt();
        std::string updatedAtStr = project.getUpdatedAt();
        sqlite3_bind_text(stmt, 3, createdAtStr.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, updatedAtStr.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fmt::print(stderr, "Error inserting project: {}\n", sqlite3_errmsg(db));
        }
    } else {
        fmt::print(stderr, "Error preparing the declaration: {}\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

std::optional<Project> ProjectManager::getProjectById(int id)
{
    sqlite3 *db = Database::getInstance().getConnection();
    // Get by ID query
    std::string sql = "SELECT * FROM projects WHERE id = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int projectId = sqlite3_column_int(stmt, 0);
            std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            std::string description = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            Project project(projectId, name, description);
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
