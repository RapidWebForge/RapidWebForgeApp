#ifndef FRONTENDGENERATOR_H
#define FRONTENDGENERATOR_H

#include "../../models/route/route.h"
#include "../../models/section/section.h"
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
    const std::vector<Section> &getViews() const;
    std::vector<Section> &getViews();
    const std::vector<Section> &getCustomComponents() const;
    std::vector<Section> &getCustomComponents();
    // Setters
    void setRoutes(const std::vector<Route> &routes);
    void setViews(const std::vector<Section> &views);
    void setCustomComponents(const std::vector<Section> &custComponents);

private:
    std::string projectPath;
    std::vector<Route> routes;
    std::vector<Section> views;
    std::vector<Section> custComponents;
    inja::Environment env;

    Component parseComponent(const nlohmann::json &componentJson);
    std::vector<Component> parseNestedComponents(const nlohmann::json &nestedJsonArray);
    void parseJson(const nlohmann::json &jsonSchema);
    bool generateView(const std::string &viewName);
    bool generateApp();
};

#endif // FRONTENDGENERATOR_H
