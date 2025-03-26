#ifndef FRONTENDGENERATOR_H
#define FRONTENDGENERATOR_H

#include "../../models/component/component.h"
#include "../../models/node-operation/nodeoperation.h"
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
    std::shared_ptr<BaseNode> parseComponent(const nlohmann::json &componentJson);
    std::vector<std::shared_ptr<BaseNode>> parseNestedComponents(
        const nlohmann::json &nestedJsonArray);
    void parseJson(const nlohmann::json &jsonSchema);
    // Funciones Auxiliares de búsqueda
    std::shared_ptr<Section> findViewByName(const std::string &viewName);
    std::shared_ptr<Section> findCustomComponentByName(const std::string &viewName);
    bool generateView(const std::string &viewName);
    bool generateCustomComponent(const std::string &custComponentName);
    // Funciones Auxiliares para modificaciones
    std::vector<NodeOperation> diffTrees(std::shared_ptr<BaseNode> &oldNode,
                                         std::shared_ptr<BaseNode> &newNode);
    void runEditorScript(const std::vector<std::string> args);
    void applyInsertion(std::shared_ptr<BaseNode> &node);
    void applyModification(std::shared_ptr<BaseNode> &node);
    void applyDeletion(std::shared_ptr<BaseNode> &node);
    std::string getFilePathForNode(std::shared_ptr<BaseNode> &node);
    std::string generateNodeFragment(std::shared_ptr<BaseNode> &node);
    void getReferenceForInsertion(std::string &referenceId,
                                  std::string &position,
                                  std::shared_ptr<BaseNode> &node);
    void applyRefactorForDeletedSection(const std::string &sectionName,
                                        const std::string &sectionType);

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
    // Prevent lost nodes
    bool isProgressSaved();
};

#endif // FRONTENDGENERATOR_H
