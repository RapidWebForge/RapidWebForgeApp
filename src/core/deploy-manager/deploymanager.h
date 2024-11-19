#ifndef DEPLOYMANAGER_H
#define DEPLOYMANAGER_H

#include <boost/process.hpp>
#include <string>

namespace bp = boost::process;

class DeployManager
{
public:
    DeployManager(const std::string projectPath, const std::string ngInxPath);
    ~DeployManager();

    void start(const std::string bunPath);
    void kill();
    void reload();

private:
    std::string projectPath;
    std::string ngInxPath;
    std::string ngInxDirectory;
    std::string configFilePath;

    bp::child runBackend(const std::string bunPath);
    bp::child runFrontend(const std::string bunPath);
    bp::child runNgInx();
    void createNginxConfig(int frontendPort, int backendPort);
};

#endif // DEPLOYMANAGER_H
