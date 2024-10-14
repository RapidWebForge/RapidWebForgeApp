#ifndef FRONTENDGENERATOR_H
#define FRONTENDGENERATOR_H

#include "../../models/route/route.h"
#include "../../models/view/view.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

class FrontendGenerator
{
public:
    FrontendGenerator(const std::string &projectPath);
    bool loadSchema();
    bool updateSchema();

    // Getters
    const std::vector<Route> &getRoutes() const;
    std::vector<Route> &getRoutes();
    const std::vector<View> &getViews() const;
    std::vector<View> &getViews();

private:
    std::string projectPath;
    std::vector<Route> routes;
    std::vector<View> views;

    void parseJson(const nlohmann::json &jsonSchema);
};

#endif // FRONTENDGENERATOR_H
