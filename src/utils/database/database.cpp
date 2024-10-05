#include "database.h"
#include <filesystem>
#include <fmt/core.h>
#include <iostream>

std::mutex Database::dbMutex;

Database &Database::getInstance(const std::string &dbName)
{
    static Database instance(dbName);
    return instance;
}

sqlite3 *Database::getConnection()
{
    return db;
}

Database::Database(const std::string &dbName)
{
    std::filesystem::path dbPath = dbName;
    std::filesystem::create_directories(dbPath.parent_path());

    // Intentar abrir la base de datos en la ruta especificada
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
