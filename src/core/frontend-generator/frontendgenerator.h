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
public:
    FrontendGenerator(const std::string &projectPath);
    // AST
    std::shared_ptr<BaseNode> getChildByType(const std::shared_ptr<BaseNode> &root,
                                             const std::string &type);
    // Schema
    bool loadSchema();
    bool updateSchema();
    // Code
    bool generateInitialFrontendCode();
    bool updateFrontendCode();
    // Getters
    const std::shared_ptr<BaseNode> &getFrontendRoot() const;
    // Prevent lost nodes
    bool isProgressSaved();

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
    bool generateView(const std::shared_ptr<Section> &view);
    bool generateCustomComponent(const std::shared_ptr<Section> &custComp);
    // Funciones Auxiliares para modificaciones
    std::vector<NodeOperation> diffTrees(std::shared_ptr<BaseNode> &oldNode,
                                         std::shared_ptr<BaseNode> &newNode);
    bool runEditorScript(const std::vector<std::string> stdArgs);
    bool applyInsertion(std::shared_ptr<BaseNode> &node);
    bool applyModification(std::shared_ptr<BaseNode> &node);
    bool applyDeletion(std::shared_ptr<BaseNode> &node);
    bool applyRefactorForDeletedSection(const std::string &sectionName,
                                        const std::string &sectionType);
    std::string getFilePathForNode(std::shared_ptr<BaseNode> &node);
    std::string generateNodeFragment(std::shared_ptr<BaseNode> &node);
    void getReferenceForInsertion(std::string &referenceId,
                                  std::string &position,
                                  std::shared_ptr<BaseNode> &node);
};

#endif // FRONTENDGENERATOR_H
