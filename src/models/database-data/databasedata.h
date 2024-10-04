#ifndef DATABASEDATA_H
#define DATABASEDATA_H

#include <string>

class DatabaseData
{
private:
    std::string server;
    std::string port;
    std::string user;
    std::string password;
    std::string databaseName;

public:
    DatabaseData();
    DatabaseData(const DatabaseData &databaseData);
    DatabaseData(const std::string &server,
                 const std::string &port,
                 const std::string &user,
                 const std::string &password,
                 const std::string &databaseName);

    // Getters
    std::string getServer() const;
    std::string getPort() const;
    std::string getUser() const;
    std::string getPassword() const;
    std::string getDatabaseName() const;

    // Setters
    void setServer(std::string server);
    void setPort(std::string port);
    void setUser(std::string user);
    void setPassword(std::string password);
    void setDatabase(std::string databaseName);
};

#endif // DATABASEDATA_H
