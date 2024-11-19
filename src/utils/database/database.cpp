#include "database.h"
#include <filesystem>
#include <fmt/core.h>
#include <iostream>
#include <map>

std::mutex Database::dbMutex;

Database &Database::getInstance(const std::string &dbName)
{
    std::lock_guard<std::mutex> lock(dbMutex);
    static std::map<std::string, Database> instances;

    auto it = instances.find(dbName);
    if (it == instances.end()) {
        it = instances
                 .emplace(std::piecewise_construct,
                          std::forward_as_tuple(dbName),
                          std::forward_as_tuple(dbName))
                 .first;
    }

    return it->second;
}

sqlite3 *Database::getConnection()
{
    return db;
}

Database::Database(const std::string &dbName)
{
    std::filesystem::path dbPath = dbName;
    std::filesystem::create_directories(dbPath.parent_path());

    if (sqlite3_open(dbName.c_str(), &db)) {
        fmt::print(stderr, "Error al abrir la base de datos '{}': {}\n", dbName, sqlite3_errmsg(db));
        db = nullptr;
    } else {
        fmt::print("Base de datos abierta exitosamente en: {} \n", dbName);
    }
}

Database::~Database()
{
    if (db) {
        sqlite3_close(db);
        fmt::print("Conexión a la base de datos cerrada.\n");
    }
}
