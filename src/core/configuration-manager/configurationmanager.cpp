#include "configurationmanager.h"
#include <QFileDialog>
#include "../../utils/database/database.h"
#include <fmt/core.h>

ConfigurationManager::ConfigurationManager()
{
    createConfigDatabase();
}

void ConfigurationManager::createConfigDatabase()
{
    sqlite3 *db = Database::getInstance("db/config.db").getConnection();
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS configuration (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            nginx_path TEXT NOT NULL,
            node_path TEXT NOT NULL,
            bun_path TEXT NOT NULL,
            mysql_path TEXT NOT NULL,
            status INTEGER NOT NULL DEFAULT 0
        );
    )";
    executeSQL(db, sql, nullptr);
}

bool ConfigurationManager::executeSQL(sqlite3 *db,
                                      const std::string &sql,
                                      const std::function<void(sqlite3_stmt *)> &bindParams,
                                      const std::function<void(sqlite3_stmt *)> &processResults) const
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

Configuration ConfigurationManager::getConfiguration() const
{
    sqlite3 *db = Database::getInstance("db/config.db").getConnection();

    std::string sql = "SELECT id, nginx_path, node_path, bun_path, mysql_path, status FROM "
                      "configuration WHERE id = 1;";

    Configuration configuration;

    executeSQL(db, sql, nullptr, [&](sqlite3_stmt *stmt) {
        int id = sqlite3_column_int(stmt, 0);
        std::string nginxPath = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        std::string nodePath = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        std::string bunPath = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        std::string mysqlPath = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        bool status = sqlite3_column_int(stmt, 5) != 0;

        configuration = Configuration(id, nginxPath, nodePath, bunPath, mysqlPath);
        configuration.setStatus(status);
        
    });

    return configuration;
}

void ConfigurationManager::setConfiguration(const Configuration &configuration)
{
    this->configuration = configuration;

    sqlite3 *db = Database::getInstance("db/config.db").getConnection();

    std::string sqlUpdateConfig = R"(
        INSERT OR REPLACE INTO configuration (id, nginx_path, node_path, bun_path, mysql_path, status) 
        VALUES (1, ?, ?, ?, ?, ?);
    )";

    std::string ngInxPath = configuration.getNgInxPath();
    std::string nodePath = configuration.getnodePath();
    std::string bunPath = configuration.getBunPath();
    std::string mysqlPath = configuration.getMysqlPath();
    bool status = configuration.getStatus();

    executeSQL(db, sqlUpdateConfig, [&](sqlite3_stmt *stmt) {
        sqlite3_bind_text(stmt, 1, ngInxPath.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, nodePath.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, bunPath.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, mysqlPath.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 5, status ? 1 : 0);
    });
}
