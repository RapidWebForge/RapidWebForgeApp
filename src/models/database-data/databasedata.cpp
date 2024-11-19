#include "databasedata.h"

// Empty constructor
DatabaseData::DatabaseData()
    : id(0)
    , server("")
    , port("")
    , user("")
    , password("")
    , databaseName("")
{}

// Constructor with all parameters
DatabaseData::DatabaseData(int id,
                           const std::string &server,
                           const std::string &port,
                           const std::string &user,
                           const std::string &password,
                           const std::string &databaseName)
    : id(id)
    , server(server)
    , port(port)
    , user(user)
    , password(password)
    , databaseName(databaseName)
{}

// Constructor with DatabaseData
DatabaseData::DatabaseData(const DatabaseData &databaseData)
    : id(databaseData.getId())
    , server(databaseData.getServer())
    , port(databaseData.getPort())
    , user(databaseData.getUser())
    , password(databaseData.getPassword())
    , databaseName(databaseData.getDatabaseName())
{}

// Getters
int DatabaseData::getId() const
{
    return id;
}

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
void DatabaseData::setServer(const std::string &server)
{
    this->server = server;
}

void DatabaseData::setPort(const std::string &port)
{
    this->port = port;
}

void DatabaseData::setUser(const std::string &user)
{
    this->user = user;
}

void DatabaseData::setPassword(const std::string &password)
{
    this->password = password;
}

void DatabaseData::setDatabase(const std::string &databaseName)
{
    this->databaseName = databaseName;
}
