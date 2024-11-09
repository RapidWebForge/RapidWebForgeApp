#include "configuration.h"

Configuration::Configuration()
    : id(0)
    , ngInxPath("")
    , bunPath("")
    , nodePath("")
    , mysqlPath("")
    , status(true) // Inicialmente en true

{}

Configuration::Configuration(int id,
                             const std::string &ngInxPath,
                             const std::string &nodePath,
                             const std::string &bunPath,
                             const std::string &mysqlPath)
    : id(id)
    , ngInxPath(ngInxPath)
    , nodePath(nodePath)
    , bunPath(bunPath)
    , mysqlPath(mysqlPath)
    , status(true) // Inicialmente en true

{}

Configuration::Configuration(const Configuration &configuration)
    : id(configuration.getId())
    , ngInxPath(configuration.getNgInxPath())
    , nodePath(configuration.getnodePath())
    , bunPath(configuration.getBunPath())
    , mysqlPath(configuration.getMysqlPath())
    , status(configuration.isStatus()) // Copiar el estado

{}

int Configuration::getId() const
{
    return id;
}

std::string Configuration::getNgInxPath() const
{
    return this->ngInxPath;
}

std::string Configuration::getnodePath() const
{
    return this->nodePath;
}

std::string Configuration::getBunPath() const
{
    return this->bunPath;
}

std::string Configuration::getMysqlPath() const
{
    return this->mysqlPath;
}

// Función para obtener el estado de pathsValidated
bool Configuration::isStatus() const
{
    return this->status;
}

void Configuration::setNgInxPath(const std::string &ngInxPath)
{
    this->ngInxPath = ngInxPath;
}

void Configuration::setnodePath(const std::string &nodePath)
{
    this->nodePath = nodePath;
}

void Configuration::setBunPath(const std::string &bunPath)
{
    this->bunPath = bunPath;
}

void Configuration::setMysqlPath(const std::string &mysqlPath)
{
    this->mysqlPath = mysqlPath;
}
// Función para establecer el estado de pathsValidated
void Configuration::setStatus(bool status)
{
    this->status = status;
}
