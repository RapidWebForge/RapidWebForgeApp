#ifndef FRONTENDGENERATOR_H
#define FRONTENDGENERATOR_H

#include <string>
#include <vector>

class FrontendGenerator
{
public:
    FrontendGenerator(const std::string &projectPath);
    bool loadSchema();
    bool updateSchema();

private:
    std::string projectPath;
};

#endif // FRONTENDGENERATOR_H
