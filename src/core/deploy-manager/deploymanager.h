#ifndef DEPLOYMANAGER_H
#define DEPLOYMANAGER_H

#include <QStringList>
#include <string>

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

    void createNginxConfig(int frontendPort, int backendPort);
};

#endif // DEPLOYMANAGER_H
