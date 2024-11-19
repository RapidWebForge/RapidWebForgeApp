#ifndef DATABASE_H
#define DATABASE_H

#include <mutex>
#include <sqlite3.h>
#include <string>

class Database
{
public:
    Database() = default;
    Database(const std::string &dbName);
    static Database &getInstance(const std::string &dbName = "db/projects.db");
    sqlite3 *getConnection();
    ~Database();

    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

private:
    sqlite3 *db = nullptr;
    static std::mutex dbMutex;
};

#endif // DATABASE_H
