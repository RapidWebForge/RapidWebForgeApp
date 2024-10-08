#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include "../../models/project/project.h"
#include "../backend-generator/backendgenerator.h"

class CodeGenerator
{
public:
    explicit CodeGenerator(const Project &projectPath);
    BackendGenerator backendGenerator;

public:
    bool createBaseBackendProject();
    bool createBaseFrontendProject();

private:
    Project project;
};

#endif // CODEGENERATOR_H
