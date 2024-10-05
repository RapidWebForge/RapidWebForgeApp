#ifndef DATABASEDATA_H
#define DATABASEDATA_H

#include <string>

class DatabaseData
{
private:
    int id;
    std::string server;
    std::string port;
    std::string user;
    std::string password;
    std::string databaseName;

public:
    DatabaseData();
    DatabaseData(const DatabaseData &databaseData);
    DatabaseData(int id,
                 const std::string &server,
                 const std::string &port,
                 const std::string &user,
                 const std::string &password,
                 const std::string &databaseName);

    // Getters
    int getId() const;
    std::string getServer() const;
    std::string getPort() const;
    std::string getUser() const;
    std::string getPassword() const;
    std::string getDatabaseName() const;

    // Setters
    void setServer(const std::string &server);
    void setPort(const std::string &port);
    void setUser(const std::string &user);
    void setPassword(const std::string &password);
    void setDatabase(const std::string &databaseName);
};

#endif // DATABASEDATA_H
