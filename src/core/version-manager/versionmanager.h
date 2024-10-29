#ifndef VERSIONMANAGER_H
#define VERSIONMANAGER_H

#include <string>

class VersionManager
{
public:
    VersionManager(const std::string &projectPath);
    void initializeRepository();
    void createVersion(const std::string &version);
    void deleteVersion(const std::string &version);

private:
    std::string projectPath;
};

#endif // VERSIONMANAGER_H
