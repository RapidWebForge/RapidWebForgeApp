#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>

class Configuration
{
private:
    int id;
    std::string ngInxPath;
    std::string nodePath;
    std::string bunPath;
    std::string mysqlPath;
    bool status; // Nuevo campo para el estado de validación

public:
    Configuration();
    Configuration(const Configuration &configuration);
    Configuration(int id,
                  const std::string &ngInxPath,
                  const std::string &nodePath,
                  const std::string &bunPath,
                  const std::string &mysqlPath);

    int getId() const;
    std::string getNgInxPath() const;
    std::string getnodePath() const;
    std::string getBunPath() const;
    std::string getMysqlPath() const;
    bool isStatus() const; // Nuevo getter para el estado

    void setNgInxPath(const std::string &ngInxPath);
    void setnodePath(const std::string &nodePath);
    void setBunPath(const std::string &bunPath);
    void setMysqlPath(const std::string &mysqlPath);
    void setStatus(bool status);
};

#endif // CONFIGURATION_H
