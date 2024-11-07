#ifndef FRONTENDGENERATOR_H
#define FRONTENDGENERATOR_H

#include "../../models/route/route.h"
#include "../../models/view/view.h"
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

class FrontendGenerator
{
public:
    FrontendGenerator(const std::string &projectPath);
    bool loadSchema();
    bool updateSchema();
    bool generateFrontendCode();
    bool updateFrontendCode();
    // Getters
    const std::vector<Route> &getRoutes() const;
    std::vector<Route> &getRoutes();
    const std::vector<View> &getViews() const;
    std::vector<View> &getViews();
    // Setters
    void setRoutes(const std::vector<Route> &routes);
    void setViews(const std::vector<View> &views);

private:
    std::string projectPath;
    std::vector<Route> routes;
    std::vector<View> views;
    inja::Environment env;

    Component parseComponent(const nlohmann::json &componentJson);
    std::vector<Component> parseNestedComponents(const nlohmann::json &nestedJsonArray);
    void parseJson(const nlohmann::json &jsonSchema);
    bool updateFrontendJson(const std::string &componentName);
    bool generateComponentBase(const std::string &componentName);
    bool generateMain();
    bool generateViews();
};

#endif // FRONTENDGENERATOR_H
