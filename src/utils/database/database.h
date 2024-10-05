#ifndef DATABASE_H
#define DATABASE_H

#include <mutex>
#include <sqlite3.h>
#include <string>

class Database
{
public:
    static Database &getInstance(const std::string &dbName = "db/projects.db");
    sqlite3 *getConnection();
    ~Database();

    // Prohibit copying and assignment
    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

private:
    Database(const std::string &dbName);
    sqlite3 *db;
    static std::mutex dbMutex;
};

#endif // DATABASE_H
