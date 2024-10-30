#ifndef VERSIONMANAGER_H
#define VERSIONMANAGER_H

#include <string>
#include <vector>

class VersionManager
{
public:
    VersionManager(const std::string &projectPath);
    void initializeRepository();
    void createVersion(const std::string &versionName);
    void deleteVersion(const std::string &versionName);
    void changeVersion(const std::string &versionName);
    void saveChanges();
    std::vector<std::string> listVersions();

private:
    std::string projectPath;
};

#endif // VERSIONMANAGER_H
