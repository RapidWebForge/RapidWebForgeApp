#include "databasedata.h"

// Empty constructor
DatabaseData::DatabaseData()
    : server("")
    , port("")
    , user("")
    , password("")
    , databaseName("")
{}

// Constructor with all parameters
DatabaseData::DatabaseData(const std::string &server,
                           const std::string &port,
                           const std::string &user,
                           const std::string &password,
                           const std::string &databaseName)
    : server(server)
    , port(port)
    , user(user)
    , password(password)
    , databaseName(databaseName)
{}

// Constructor with DatabaseData
DatabaseData::DatabaseData(const DatabaseData &databaseData)
    : server(databaseData.getServer())
    , port(databaseData.getPort())
    , user(databaseData.getUser())
    , password(databaseData.getPassword())
    , databaseName(databaseData.getDatabaseName())
{}

// Getters
std::string DatabaseData::getServer() const
{
    return server;
}

std::string DatabaseData::getPort() const
{
    return port;
}

std::string DatabaseData::getUser() const
{
    return user;
}

std::string DatabaseData::getPassword() const
{
    return password;
}

std::string DatabaseData::getDatabaseName() const
{
    return databaseName;
}

// Setters
void DatabaseData::setServer(std::string server)
{
    this->server = server;
}

void DatabaseData::setPort(std::string port)
{
    this->port = port;
}

void DatabaseData::setUser(std::string user)
{
    this->user = user;
}

void DatabaseData::setPassword(std::string password)
{
    this->password = password;
}

void DatabaseData::setDatabase(std::string databaseName)
{
    this->databaseName = databaseName;
}
