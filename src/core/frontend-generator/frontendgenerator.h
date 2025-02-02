#ifndef FRONTENDGENERATOR_H
#define FRONTENDGENERATOR_H

#include "../../models/component/component.h"
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
    const std::vector<std::shared_ptr<Section>> &getViews() const;
    const std::vector<std::shared_ptr<Section>> &getCustomComponents() const;
    // Setters
    void setRoutes(const std::vector<Route> &routes);
    void setViews(const std::vector<std::shared_ptr<Section>> &views);
    void setCustomComponents(const std::vector<std::shared_ptr<Section>> &custComponents);

    // Process functions
    void processSection(const std::shared_ptr<Section> &section, nlohmann::json &jsonArray);
    nlohmann::json processSectionToJson(const std::shared_ptr<Section> &section);

private:
    std::string projectPath;
    std::vector<Route> routes;
    std::vector<std::shared_ptr<Section>> views;
    std::vector<std::shared_ptr<Section>> custComponents;
    inja::Environment env;

    std::shared_ptr<BaseNode> parseComponent(const nlohmann::json &componentJson);
    std::vector<std::shared_ptr<BaseNode>> parseNestedComponents(
        const nlohmann::json &nestedJsonArray);
    void parseJson(const nlohmann::json &jsonSchema);

    void initializeCustomComponentsCache();

    bool generateView(const std::string &viewName);
    bool generateCustomComponent(const std::string &viewName);
    bool generateApp();
};

#endif // FRONTENDGENERATOR_H
