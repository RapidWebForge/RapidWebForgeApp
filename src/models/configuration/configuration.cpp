#include "configuration.h"

Configuration::Configuration()
    : id(0)
    , ngInxPath("")
    , bunPath("")
    , nodePath("")
    , mysqlPath("")
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
{}

Configuration::Configuration(const Configuration &configuration)
    : id(configuration.getId())
    , ngInxPath(configuration.getNgInxPath())
    , nodePath(configuration.getnodePath())
    , bunPath(configuration.getBunPath())
    , mysqlPath(configuration.getMysqlPath())
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
