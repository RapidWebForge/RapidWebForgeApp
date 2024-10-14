#ifndef FRONTENDGENERATOR_H
#define FRONTENDGENERATOR_H

#include "../../models/route/route.h"
#include "../../models/view/view.h"
#include <string>
#include <vector>

class FrontendGenerator
{
public:
    FrontendGenerator(const std::string &projectPath);
    bool loadSchema();
    bool updateSchema();

    const std::vector<Route> &getRoutes() const;
    const std::vector<View> &getViews() const;

private:
    std::string projectPath;
    std::vector<Route> routes;
    std::vector<View> views;

    void parseJson(const nlohmann::json &jsonSchema);
};

#endif // FRONTENDGENERATOR_H
