#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include "../../models/project/project.h"
#include "../backend-generator/backendgenerator.h"
#include "../frontend-generator/frontendgenerator.h"
#include <nlohmann/json.hpp>

class CodeGenerator
{
public:
    explicit CodeGenerator(const Project &project);
    BackendGenerator backendGenerator;
    FrontendGenerator frontendGenerator;

    bool createBaseBackendProject();  // Create backend project
    bool createBaseFrontendProject(); // Create frontend project

    std::string getProjectPath() const;

private:
    bool createDirectory(const std::string &path);
    bool unzipFile(const std::string &zipPath, const std::string &extractPath);
    bool createJsonFile(const std::string &filePath, const nlohmann::json &jsonData);

    Project project;
};

#endif // CODEGENERATOR_H
