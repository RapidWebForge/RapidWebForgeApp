#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include "../backend-generator/backendgenerator.h"

class CodeGenerator
{
public:
    explicit CodeGenerator(const std::string &projectPath);
    BackendGenerator backendGenerator;
};

#endif // CODEGENERATOR_H
