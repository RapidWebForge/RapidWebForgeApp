#ifndef FRONTENDGENERATOR_H
#define FRONTENDGENERATOR_H

#include "../../models/component/component.h"
#include "../../models/section/section.h"
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

class FrontendGenerator
{
private:
    std::string projectPath;
    std::shared_ptr<BaseNode> frontendRoot;
    std::shared_ptr<BaseNode> oldRoot;
    inja::Environment env;

    void initializeCustomComponentsCache();

    void generateCodeForNode(const std::shared_ptr<BaseNode> &node);
    // Schema
    void parseJson(const nlohmann::json &jsonSchema);
    // Auxiliar
    std::shared_ptr<Section> findViewByName(const std::string &viewName);
    bool generateView(const std::string &viewName);
    bool generateCustomComponent(const std::string &custComponentName);

public:
    FrontendGenerator(const std::string &projectPath);
    // AST
    const std::shared_ptr<BaseNode> getMainNode(const std::string &nodeName);
    // Schema
    bool loadSchema();
    bool updateSchema();
    // Code
    bool generateFrontendCode();
    bool updateFrontendCode();
    // Getters
    const std::shared_ptr<BaseNode> &getFrontendRoot() const;
};

#endif // FRONTENDGENERATOR_H
