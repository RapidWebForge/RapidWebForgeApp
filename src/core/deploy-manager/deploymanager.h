#ifndef DEPLOYMANAGER_H
#define DEPLOYMANAGER_H

#include <QStringList>
#include <string>

class DeployManager
{
public:
    DeployManager(const std::string &projectPath,
                  const std::string &ngInxPath,
                  const std::string &bunPath);
    ~DeployManager();

    void start();
    void kill();
    void reload();

private:
    std::string projectPath;
    std::string ngInxPath;
    std::string bunPath;

    std::string ngInxDirectory;
    std::string configFilePath;

    void createNginxConfig(int frontendPort, int backendPort);
};

#endif // DEPLOYMANAGER_H
