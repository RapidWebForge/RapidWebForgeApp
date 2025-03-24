#include "frontendgenerator.h"
#include <QDebug>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include "../../models/component-type/componenttype.h"
#include "../../models/generic-node/genericnode.h"
#include "../../models/time-chrono/timechrono.h"
#include "../../utils/file/fileutiils.h"
#include "../../utils/render_callback/rendercallback.h"
#include <boost/process.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fmt/core.h>
#include <fstream>

FrontendGenerator::FrontendGenerator(const std::string &projectPath)
    : projectPath(projectPath)
    , frontendRoot(std::make_shared<GenericNode>("FrontendRoot"))
{
    try {
        env.add_callback("render_component", [this](inja::Arguments &args) -> std::string {
            return RenderCallback::renderComponentCallback(this->env, args);
        });
        env.add_callback("render_imports", 1, [this](inja::Arguments &args) -> std::string {
            return RenderCallback::renderImportsCallback(this->env, args);
        });
        env.add_callback("render_states", 1, [this](inja::Arguments &args) -> std::string {
            return RenderCallback::renderStatesCallback(this->env, args);
        });
        env.add_callback("render_handles", 1, [this](inja::Arguments &args) -> std::string {
            return RenderCallback::renderHandleFoosCallback(this->env, args);
        });
        env.add_callback("render_requests", 1, [this](inja::Arguments &args) -> std::string {
            return RenderCallback::renderRequestsCallback(this->env, args);
        });
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error adding callback: {}\n", e.what());
    }
}

void FrontendGenerator::initializeCustomComponentsCache()
{
    RenderCallback::customComponentsCache.clear();

    // Encontrar el nodo 'CustomComponents' en el AST
    auto customComponentsNode = getMainNode("CustomComponents");

    if (!customComponentsNode) {
        fmt::print(stderr, "Error: No 'CustomComponents' node found in the AST.\n");
        return;
    }

    // Recorrer los hijos del nodo 'CustomComponents'
    for (const auto &customComponent : customComponentsNode->getChildren()) {
        // Obtener el nombre de la sección
        std::string name = static_cast<Section *>(customComponent.get())->getName();

        // Guardar en la caché
        RenderCallback::customComponentsCache.insert(name);
    }

    // Mostrar mensaje de éxito
    fmt::print("Custom components cache initialized with {} items.\n",
               RenderCallback::customComponentsCache.size());
}

std::chrono::system_clock::time_point parseDateTime(const std::string &dateTimeStr)
{
    std::tm tm = {};
    std::istringstream ss(dateTimeStr);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::shared_ptr<BaseNode> FrontendGenerator::parseComponent(const nlohmann::json &componentJson)
{
    boost::uuids::uuid id;

    if (componentJson.contains("id") && componentJson["id"].is_string()) {
        boost::uuids::string_generator gen;
        id = gen(componentJson["id"].get<std::string>());
    }

    std::chrono::system_clock::time_point createdOn, updatedOn;

    if (componentJson.contains("createdOn") && componentJson["createdOn"].is_string()) {
        createdOn = parseDateTime(componentJson["createdOn"].get<std::string>());
    }

    if (componentJson.contains("updatedOn") && componentJson["updatedOn"].is_string()) {
        updatedOn = parseDateTime(componentJson["updatedOn"].get<std::string>());
    }

    // If it's a component
    if (componentJson.contains("type")) {
        auto component = std::make_shared<Component>(id, createdOn, updatedOn);

        // Parse type
        if (componentJson.contains("type") && componentJson["type"].is_string()) {
            component->setType(stringToComponentType(componentJson["type"].get<std::string>()));
        }

        // Parse props
        std::map<std::string, std::string> props;

        if (componentJson.contains("props") && componentJson["props"].is_object()) {
            for (auto it = componentJson["props"].begin(); it != componentJson["props"].end();
                 ++it) {
                if (it.value().is_string()) {
                    props[it.key()] = it.value().get<std::string>();
                } else {
                    fmt::print(stderr,
                               "Error: 'props' value for '{}' must be a string.\n",
                               it.key());
                }
            }
        }

        component->setProps(props);

        // Verificar si hay componentes anidados
        if (componentJson.contains("nestedComponents")) {
            auto nestedComponents = parseNestedComponents(componentJson["nestedComponents"]);
            for (const auto &nestedComponent : nestedComponents) {
                component->addChild(nestedComponent);
            }
        }

        return component;
    } else if (componentJson.contains("name") && componentJson["name"].is_string()) {
        std::string name = componentJson["name"].get<std::string>();
        auto section = std::make_shared<Section>(name, id, createdOn, updatedOn);
        return section;
    }
}

std::vector<std::shared_ptr<BaseNode>> FrontendGenerator::parseNestedComponents(
    const nlohmann::json &nestedJsonArray)
{
    std::vector<std::shared_ptr<BaseNode>> nestedComponents;

    for (const auto &nestedJson : nestedJsonArray) {
        auto nestedComponent = parseComponent(nestedJson);
        if (nestedComponent) {
            nestedComponents.push_back(nestedComponent);
        }
    }
    return nestedComponents;
}

// void printNodeTree(const std::shared_ptr<BaseNode> &node, int depth = 0)
// {
//     if (!node)
//         return;

//     QString indent = QString(" ").repeated(depth * 2);

//     if (auto component = std::dynamic_pointer_cast<Component>(node)) {
//         qDebug().noquote() << indent + "  (ID: "
//                                   + QString::fromStdString(
//                                       boost::uuids::to_string(component->getId()))
//                                   + ")";
//         qDebug().noquote() << indent + "  (Type: "
//                                   + QString::fromStdString(
//                                       componentTypeToString(component->getType()))
//                                   + ")";

//     } else if (auto section = std::dynamic_pointer_cast<Section>(node)) {
//         qDebug().noquote() << indent + "- " + QString::fromStdString(section->getName());
//     } else {
//         qDebug().noquote() << indent + "- " + QString::fromStdString(node->getNodeType());
//     }

//     // Recursively print children
//     for (const auto &child : node->getChildren()) {
//         printNodeTree(child, depth + 1);
//     }
// }

std::shared_ptr<BaseNode> cloneNode(const std::shared_ptr<BaseNode> &node)
{
    if (!node) {
        return nullptr;
    }

    return node->clone();
}

void FrontendGenerator::parseJson(const nlohmann::json &jsonSchema)
{
    // Main nodes by categories
    auto customComponentsNode = std::make_shared<GenericNode>("CustomComponents");
    auto viewsNode = std::make_shared<GenericNode>("Views");

    // Categories on frontendRoot
    this->frontendRoot->addChild(customComponentsNode);
    this->frontendRoot->addChild(viewsNode);

    // Parse custom components
    for (const auto &custComponentJson : jsonSchema["custom"]) {
        boost::uuids::uuid id;

        if (custComponentJson.contains("id") && custComponentJson["id"].is_string()) {
            boost::uuids::string_generator gen;
            id = gen(custComponentJson["id"].get<std::string>());
        }

        std::chrono::system_clock::time_point createdOn, updatedOn;

        if (custComponentJson.contains("createdOn") && custComponentJson["createdOn"].is_string()) {
            createdOn = parseDateTime(custComponentJson["createdOn"].get<std::string>());
        }

        if (custComponentJson.contains("updatedOn") && custComponentJson["updatedOn"].is_string()) {
            updatedOn = parseDateTime(custComponentJson["updatedOn"].get<std::string>());
        }

        std::string name = custComponentJson["name"].get<std::string>();

        auto customSection = std::make_shared<Section>(name, id, createdOn, updatedOn);

        // Parse components inside the section
        // if (custComponentJson.contains("components") && custComponentJson["components"].is_array())
        for (const auto &componentJson : custComponentJson["components"]) {
            auto componentNode = parseComponent(componentJson);
            if (componentNode) {
                customSection->addChild(componentNode);
            }
        }

        // Agregar la sección al nodo principal de custom components
        customComponentsNode->addChild(customSection);
    }

    // Parse views
    for (const auto &viewJson : jsonSchema["views"]) {
        boost::uuids::uuid id;

        if (viewJson.contains("id") && viewJson["id"].is_string()) {
            boost::uuids::string_generator gen;
            id = gen(viewJson["id"].get<std::string>());
        }

        std::chrono::system_clock::time_point createdOn, updatedOn;

        if (viewJson.contains("createdOn") && viewJson["createdOn"].is_string()) {
            createdOn = parseDateTime(viewJson["createdOn"].get<std::string>());
        }

        if (viewJson.contains("updatedOn") && viewJson["updatedOn"].is_string()) {
            updatedOn = parseDateTime(viewJson["updatedOn"].get<std::string>());
        }

        std::string name = viewJson["name"].get<std::string>();
        std::string path = viewJson["path"].get<std::string>();

        auto viewSection = std::make_shared<Section>(name, path, id, createdOn, updatedOn);

        // Parse components inside the section
        // if (viewJson.contains("components") && viewJson["components"].is_array())
        for (const auto &componentJson : viewJson["components"]) {
            auto componentNode = parseComponent(componentJson);
            if (componentNode) {
                viewSection->addChild(componentNode);
            }
        }

        // Agregar la vista al nodo principal de views
        viewsNode->addChild(viewSection);
    }

    // Generate clone
    oldRoot = cloneNode(frontendRoot);

    // qDebug() << "FrontendRoot";
    // printNodeTree(frontendRoot);
    // qDebug() << "OldRoot";
    // printNodeTree(oldRoot);
}

bool FrontendGenerator::loadSchema()
{
    std::ifstream file(projectPath + "/frontend.json");
    if (!file.is_open()) {
        fmt::print(stderr, "Unable to open JSON file: {}/frontend.json\n", projectPath);

        // Create frontend.json if not exists
        nlohmann::json frontendJson;
        frontendJson["views"] = nlohmann::json::array();
        frontendJson["custom"] = nlohmann::json::array();

        std::ofstream jsonFile(projectPath + "/frontend.json");
        if (!jsonFile.is_open()) {
            fmt::print(stderr, "Failed to create frontend.json\n");
            return false;
        }

        jsonFile << frontendJson.dump(2);
        jsonFile.close();
        return false;
    }

    nlohmann::json jsonSchema;
    try {
        file >> jsonSchema;
    } catch (const nlohmann::json::parse_error &e) {
        fmt::print(stderr, "Error parsing JSON: {}\n", e.what());
        return false;
    }

    parseJson(jsonSchema); // JSON to AST
    return true;
}

bool allowsNestedComponents(ComponentType type)
{
    return type == ComponentType::Form || type == ComponentType::HorizontalLayout
           || type == ComponentType::VerticalLayout || type == ComponentType::ModelLayout;
}

nlohmann::json processComponentToJson(const std::shared_ptr<Component> &component)
{
    nlohmann::json componentJson;
    componentJson["type"] = componentTypeToString(component->getType());
    componentJson["id"] = boost::uuids::to_string(component->getId());
    componentJson["createdOn"] = timePointToString(component->getCreatedOn());
    componentJson["updatedOn"] = timePointToString(component->getUpdatedOn());

    // Procesa las propiedades del componente
    nlohmann::json propsJson;
    for (const auto &prop : component->getProps()) {
        propsJson[prop.first] = prop.second;
    }
    componentJson["props"] = propsJson;

    // Si el componente permite elementos anidados, se procesan de forma recursiva
    if (component->isAllowingItems()) {
        componentJson["nestedComponents"] = nlohmann::json::array();
        for (const auto &child : component->getChildren()) {
            // Se asume que los hijos también son componentes (o se pueden procesar similarmente)
            if (auto nestedComponent = std::dynamic_pointer_cast<Component>(child)) {
                componentJson["nestedComponents"].push_back(processComponentToJson(nestedComponent));
            } else if (auto nestedSection = std::dynamic_pointer_cast<Section>(child)) {
                // Si en algún caso se manejan sub-secciones
                nlohmann::json subSectionJson;
                subSectionJson["name"] = nestedSection->getName();
                subSectionJson["createdOn"] = timePointToString(nestedSection->getCreatedOn());
                subSectionJson["updatedOn"] = timePointToString(nestedSection->getUpdatedOn());
                subSectionJson["id"] = boost::uuids::to_string(nestedSection->getId());

                componentJson["nestedComponents"].push_back(subSectionJson);
            }
        }
    }
    return componentJson;
}

nlohmann::json processSectionToJson(const std::shared_ptr<Section> &section)
{
    nlohmann::json sectionJson;
    sectionJson["name"] = section->getName();
    sectionJson["id"] = boost::uuids::to_string(section->getId());
    sectionJson["createdOn"] = timePointToString(section->getCreatedOn());
    sectionJson["updatedOn"] = timePointToString(section->getUpdatedOn());
    sectionJson["components"] = nlohmann::json::array();

    if (!section->getPath().empty())
        sectionJson["path"] = section->getPath();

    for (const auto &child : section->getChildren()) {
        if (auto component = std::dynamic_pointer_cast<Component>(child)) {
            sectionJson["components"].push_back(processComponentToJson(component));
        } else if (auto subSection = std::dynamic_pointer_cast<Section>(child)) {
            nlohmann::json subSectionJson;
            subSectionJson["name"] = subSection->getName();
            subSectionJson["createdOn"] = timePointToString(subSection->getCreatedOn());
            subSectionJson["updatedOn"] = timePointToString(subSection->getUpdatedOn());
            subSectionJson["id"] = boost::uuids::to_string(subSection->getId());

            sectionJson["components"].push_back(subSectionJson);
        }
    }

    return sectionJson;
}

// AST
const std::shared_ptr<BaseNode> FrontendGenerator::getMainNode(const std::string &nodeName)
{
    auto it = std::find_if(frontendRoot->getChildren().begin(),
                           frontendRoot->getChildren().end(),
                           [&nodeName](const std::shared_ptr<BaseNode> &node) {
                               return node->getNodeType() == nodeName;
                           });

    if (it != frontendRoot->getChildren().end()) {
        return *it; // Devuelve el nodo encontrado
    }

    return nullptr; // No se encontró el nodo
}

// Updating

bool FrontendGenerator::updateSchema()
{
    nlohmann::json jsonSchema;

    // printNodeTree(frontendRoot);

    auto viewsNode = getMainNode("Views");

    // Create the views
    if (viewsNode) {
        // Convert to section to use getChildren()
        auto sectionNode = std::dynamic_pointer_cast<GenericNode>(viewsNode);
        if (sectionNode) {
            // Create views json
            jsonSchema["views"] = nlohmann::json::array();

            for (const auto &view : sectionNode->getChildren()) {
                auto section = std::dynamic_pointer_cast<Section>(view);
                if (section) {
                    nlohmann::json viewJson = processSectionToJson(section);

                    jsonSchema["views"].push_back(viewJson);
                }
            }
        } else {
            fmt::print(stderr, "Error: 'Views' node is not a GenericNode.\n");
        }
    } else {
        fmt::print(stderr, "Error: 'Views' node not found in the AST.\n");
    }

    auto customComponentsNode = getMainNode("CustomComponents");

    if (customComponentsNode) {
        // Convert to section to use getChildren()
        auto sectionNode = std::dynamic_pointer_cast<GenericNode>(customComponentsNode);
        if (sectionNode) {
            // Create views json
            jsonSchema["custom"] = nlohmann::json::array();

            for (const auto &view : sectionNode->getChildren()) {
                auto section = std::dynamic_pointer_cast<Section>(view);
                if (section) {
                    nlohmann::json viewJson = processSectionToJson(section);

                    jsonSchema["custom"].push_back(viewJson);
                }
            }
        } else {
            fmt::print(stderr, "Error: 'Views' node is not a GenericNode.\n");
        }
    } else {
        fmt::print(stderr, "Error: 'Views' node not found in the AST.\n");
    }

    // Save the frontend.json file
    std::string filePath = projectPath + "/frontend.json";
    std::ofstream jsonFile(filePath);
    if (!jsonFile.is_open()) {
        fmt::print(stderr, "Failed to open the file for writing: {}\n", filePath);
        return false;
    }

    // std::cout << jsonSchema.dump(2) << std::endl; // Imprime el JSON en la consola antes de guardar

    jsonFile << jsonSchema.dump(2);
    jsonFile.close();

    return true;
}

std::shared_ptr<Section> FrontendGenerator::findViewByName(const std::string &viewName)
{
    // Encontrar el nodo "Views" en el AST
    auto viewsNode = getMainNode("Views");
    if (!viewsNode) {
        fmt::print(stderr, "Error: 'Views' node not found in the AST.\n");
        return nullptr;
    }

    // Convertir el nodo "Views" a GenericNode
    auto sectionNode = std::dynamic_pointer_cast<GenericNode>(viewsNode);
    if (!sectionNode) {
        fmt::print(stderr, "Error: 'Views' node is not a GenericNode.\n");
        return nullptr;
    }

    // Usar std::find_if para buscar la vista por nombre
    auto it = std::find_if(sectionNode->getChildren().begin(),
                           sectionNode->getChildren().end(),
                           [&viewName](const std::shared_ptr<BaseNode> &node) {
                               auto section = std::dynamic_pointer_cast<Section>(node);
                               return section && section->getName() == viewName;
                           });

    // Verificar si se encontró la vista
    if (it != sectionNode->getChildren().end()) {
        return std::dynamic_pointer_cast<Section>(*it);
    }

    return nullptr; // No se encontró la vista
}

std::shared_ptr<Section> FrontendGenerator::findCustomComponentByName(const std::string &viewName)
{
    // Encontrar el nodo "CustomComponents" en el AST
    auto custCompNode = getMainNode("CustomComponents");
    if (!custCompNode) {
        fmt::print(stderr, "Error: 'CustomComponents' node not found in the AST.\n");
        return nullptr;
    }

    // Convertir el nodo "CustomComponents" a GenericNode
    auto sectionNode = std::dynamic_pointer_cast<GenericNode>(custCompNode);
    if (!sectionNode) {
        fmt::print(stderr, "Error: 'CustomComponents' node is not a GenericNode.\n");
        return nullptr;
    }

    // Usar std::find_if para buscar la vista por nombre
    auto it = std::find_if(sectionNode->getChildren().begin(),
                           sectionNode->getChildren().end(),
                           [&viewName](const std::shared_ptr<BaseNode> &node) {
                               auto section = std::dynamic_pointer_cast<Section>(node);
                               return section && section->getName() == viewName;
                           });

    // Verificar si se encontró la vista
    if (it != sectionNode->getChildren().end()) {
        return std::dynamic_pointer_cast<Section>(*it);
    }

    return nullptr; // No se encontró la vista
}

bool FrontendGenerator::generateView(const std::string &viewName)
{
    nlohmann::json data;

    // Inserta el nombre del componente en el contexto de Inja

    auto view = findViewByName(viewName);

    if (!view) {
        qDebug() << "View " << viewName << " wasn't found\n";
        return false;
    }

    auto viewNode = std::dynamic_pointer_cast<Section>(view);

    if (!viewNode) {
        qDebug() << "View " << viewName << " couldn't be casted\n";
        return false;
    }

    data = processSectionToJson(viewNode);

    // qDebug().noquote() << data.dump(2);

    std::string templatePath = ":/inja/frontend/view";

    // Cargar el template
    QFile file(QString::fromStdString(templatePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmt::print(stderr, "Unable to open template file from resource: {}\n", templatePath);
        return false;
    }

    QTextStream in(&file);
    QString templateContent = in.readAll();
    file.close();

    std::string templateString = templateContent.toStdString();
    std::string outputPath = projectPath + "/frontend/src/views/" + viewName + ".tsx";

    try {
        // Renderizar con Inja usando el contenido del archivo como una cadena
        std::string result = env.render(templateString, data);
        FileUtils::writeFile(outputPath, result);
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error generating view {}: {}\n", viewName, e.what());
        return false;
    }

    fmt::print("View generated successfully for {}\n", viewName);
    return true;
}

bool FrontendGenerator::generateCustomComponent(const std::string &custComponentName)
{
    nlohmann::json data;

    auto custComp = findCustomComponentByName(custComponentName);

    if (!custComp) {
        qDebug() << "Custom Component " << custComponentName << " wasn't found\n";
        return false;
    }

    auto custCompNode = std::dynamic_pointer_cast<Section>(custComp);

    if (!custCompNode) {
        qDebug() << "Custom Component " << custComponentName << " couldn't be casted\n";
        return false;
    }

    data = processSectionToJson(custCompNode);

    std::string templatePath = ":/inja/frontend/view";

    // Cargar el template
    QFile file(QString::fromStdString(templatePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmt::print(stderr, "Unable to open template file from resource: {}\n", templatePath);
        return false;
    }

    QTextStream in(&file);
    QString templateContent = in.readAll();
    file.close();

    std::string templateString = templateContent.toStdString();
    std::string outputPath = projectPath + "/frontend/src/components/" + custComponentName + ".tsx";

    try {
        // Renderizar con Inja usando el contenido del archivo como una cadena
        std::string result = env.render(templateString, data);
        FileUtils::writeFile(outputPath, result);
    } catch (const std::exception &e) {
        fmt::print(stderr,
                   "Error generating custom component {}: {}\n",
                   custComponentName,
                   e.what());
        return false;
    }

    fmt::print("Custom component generated successfully for {}\n", custComponentName);
    return true;
}

bool FrontendGenerator::generateFrontendCode()
{
    initializeCustomComponentsCache();
    nlohmann::json data;
    data["routes"] = nlohmann::json::array();

    auto viewsNode = getMainNode("Views");

    auto viewsPtr = std::dynamic_pointer_cast<GenericNode>(viewsNode);

    for (const auto &view : viewsPtr->getChildren()) {
        auto section = std::dynamic_pointer_cast<Section>(view);
        if (section) {
            if (!generateView(section->getName())) {
                fmt::print(stderr, "Failed to generate component base for {}\n", section->getName());
                return false;
            } else {
                nlohmann::json routeJson;
                routeJson["component"] = section->getName();
                routeJson["path"] = section->getPath();
                data["routes"].push_back(routeJson);
            }
        }
    }

    auto customComponentsNode = getMainNode("CustomComponents");

    auto customComponentPtr = std::dynamic_pointer_cast<GenericNode>(customComponentsNode);

    for (const auto &custComp : customComponentPtr->getChildren()) {
        auto section = std::dynamic_pointer_cast<Section>(custComp);
        if (section) {
            if (!generateCustomComponent(section->getName())) {
                fmt::print(stderr, "Failed to generate component base for {}\n", section->getName());
                return false;
            }
        }
    }

    std::string templatePath = ":/inja/frontend/app";

    QFile file(QString::fromStdString(templatePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmt::print(stderr, "Unable to open template file from resource: {}\n", templatePath);
        return false;
    }

    QTextStream in(&file);
    QString templateContent = in.readAll();
    file.close();

    std::string templateString = templateContent.toStdString();

    std::string outputPath = projectPath + "/frontend/src/App.tsx";

    try {
        std::string result = env.render(templateString, data);
        FileUtils::writeFile(outputPath, result);
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error generating App.tsx: {}\n", e.what());
        return false;
    }

    return true;
}

// Funciones Auxiliares para modificaciones

std::vector<NodeOperation> FrontendGenerator::diffTrees(std::shared_ptr<BaseNode> &oldNode,
                                                        std::shared_ptr<BaseNode> &newNode)
{
    std::vector<NodeOperation> ops;

    // Si el nodo es uno de los contenedores de alto nivel, ignóralo y recorre sus hijos secuencialmente.
    std::string type = newNode->getNodeType();
    if (type == "FrontendRoot" || type == "Views" || type == "CustomComponents") {
        auto oldChildren = oldNode->getChildren();
        auto newChildren = newNode->getChildren();

        // Recorrer hijos en orden
        for (size_t i = 0; i < newChildren.size(); ++i) {
            if (i < oldChildren.size()) {
                auto childOps = diffTrees(oldChildren[i], newChildren[i]);
                ops.insert(ops.end(), childOps.begin(), childOps.end());
            } else {
                ops.push_back(NodeOperation(OperationType::Insert, newChildren[i]));
            }
        }
        // Si hay más hijos en el antiguo que en el nuevo, se consideran eliminaciones.
        if (oldChildren.size() > newChildren.size()) {
            for (size_t i = newChildren.size(); i < oldChildren.size(); ++i) {
                ops.push_back(NodeOperation(OperationType::Delete, oldChildren[i]));
            }
        }
        return ops;
    }

    // Para los demás nodos, si tienen el mismo ID y difieren en contenido, se marca como modificación.
    if (oldNode->getId() == newNode->getId() && oldNode->isDifferentFrom(newNode)) {
        ops.push_back(NodeOperation(OperationType::Modify, newNode));
    }

    // Ahora, mapear hijos antiguos por ID para comparar fácilmente.
    std::unordered_map<std::string, std::shared_ptr<BaseNode>> oldChildrenMap;
    for (auto child : oldNode->getChildren()) {
        std::string id = boost::uuids::to_string(child->getId());
        oldChildrenMap[id] = child;
    }

    // Recorremos los hijos del nuevo nodo usando el mapa.
    for (auto newChild : newNode->getChildren()) {
        std::string id = boost::uuids::to_string(newChild->getId());
        if (oldChildrenMap.find(id) != oldChildrenMap.end()) {
            auto childOps = diffTrees(oldChildrenMap[id], newChild);
            ops.insert(ops.end(), childOps.begin(), childOps.end());
            oldChildrenMap.erase(id);
        } else {
            ops.push_back(NodeOperation(OperationType::Insert, newChild));
        }
    }

    // Los nodos que quedaron en oldChildrenMap se consideran eliminaciones.
    for (auto &pair : oldChildrenMap) {
        ops.push_back(NodeOperation(OperationType::Delete, pair.second));
    }

    return ops;
}

bool FrontendGenerator::updateFrontendCode()
{
    std::vector<NodeOperation> operations = diffTrees(oldRoot, frontendRoot);

    if (operations.empty()) {
        qDebug() << "No changes detected, skipping frontend generation.";
        return true;
    }

    // Aplicar cada operación de forma incremental
    for (auto op : operations) {
        switch (op.type) {
        case OperationType::Insert:
            // Ubicar posición mediante op.node->id (data-id) y generar fragmento
            applyInsertion(op.node);
            break;
        case OperationType::Modify:
            // Buscar en el archivo el fragmento con data-id y actualizarlo
            applyModification(op.node);
            break;
        case OperationType::Delete:
            // Eliminar el fragmento con el data-id del nodo eliminado
            applyDeletion(op.node);
            break;
        }
    }

    // Actualizar archivos que dependen de cambios a nivel de Section (ej. App.tsx, etc.)
    // updateDependentFiles();

    if (!updateSchema()) {
        qDebug() << "Error on Updating Schema";
        return false;
    }

    // Actualizar el oldRoot para futuras comparaciones
    oldRoot = cloneNode(frontendRoot);
    return true;
}

void FrontendGenerator::applyInsertion(std::shared_ptr<BaseNode> &node)
{
    // Si se quiere agregar un nuevo view o custom component
    if (node->getNodeType() == "Section" && node->getParent()->getNodeType() != "Section"
        && node->getParent()->getNodeType() != "Component") {
        auto section = std::dynamic_pointer_cast<Section>(node);
        if (!section) {
            fmt::print(stderr, "Error: nodo identificado como section pero falla el cast.\n");
            return;
        }
        if (section->getPath().empty()) {
            if (generateCustomComponent(section->getName()))
                return;
        } else {
            if (generateView(section->getName()))
                return;
        }
    } else {
        // Determinar en qué archivo se debe insertar el nodo
        std::string filePath = getFilePathForNode(node);

        // Generar el fragmento de código usando la plantilla Inja
        std::string jsxFragment = generateNodeFragment(node);

        QString safeJsx
            = QString::fromStdString(jsxFragment).replace("\\", "\\\\").replace("\"", "\\\"");
        std::string finalJsxArg = "\"" + safeJsx.toStdString() + "\"";

        // Obtener la el dataId de referencia y la posición correspondiente de este id
        std::string referenceId, position;
        getReferenceForInsertion(referenceId, position, node);

        // Nuevo método con Babel

        QString resourcePath = ":/babel/editor";
        QFile resourceFile(resourcePath);
        if (!resourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            fmt::print(stderr, "❌ Unable to open resource: {}\n", resourcePath.toStdString());
            return;
        }

        QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        QString tempFilePath = tempPath + "/editor.js";

        QFile tempFile(tempFilePath);
        if (tempFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            tempFile.write(resourceFile.readAll());
            tempFile.close();
        } else {
            fmt::print(stderr, "❌ Unable to write temporary editor.js\n");
            return;
        }

        // Correr el comando externo con Node.js

        namespace bp = boost::process;

        bp::environment env = boost::this_process::environment();
        env["NODE_PATH"] = "/usr/local/lib/node_modules"; // Ajusta según tu sistema

        bp::child c("/usr/local/bin/node",
                    tempFilePath.toStdString(),
                    filePath,
                    "insert",
                    referenceId,
                    position,
                    finalJsxArg,
                    env);
        c.wait();
    }
}

void FrontendGenerator::applyModification(std::shared_ptr<BaseNode> &node)
{
    // Si la modificación es en un view o custom component
    if (node->getNodeType() == "Section")
        return;

    // Si la modificación es en un Component

    // Determinar en qué archivo se debe insertar el nodo
    std::string filePath = getFilePathForNode(node);

    // Generar el fragmento de código usando la plantilla Inja
    std::string jsxFragment = generateNodeFragment(node);

    QString safeJsx = QString::fromStdString(jsxFragment).replace("\\", "\\\\").replace("\"", "\\\"");
    std::string finalJsxArg = "\"" + safeJsx.toStdString() + "\"";

    // Obtener la el dataId
    std::string dataId = boost::uuids::to_string(node->getId());

    QString resourcePath = ":/babel/editor";
    QFile resourceFile(resourcePath);
    if (!resourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmt::print(stderr, "❌ Unable to open resource: {}\n", resourcePath.toStdString());
        return;
    }

    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString tempFilePath = tempPath + "/editor.js";

    QFile tempFile(tempFilePath);
    if (tempFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        tempFile.write(resourceFile.readAll());
        tempFile.close();
    } else {
        fmt::print(stderr, "❌ Unable to write temporary editor.js\n");
        return;
    }

    // Correr el comando externo con Node.js

    namespace bp = boost::process;

    bp::environment env = boost::this_process::environment();
    env["NODE_PATH"] = "/usr/local/lib/node_modules"; // Ajusta según tu sistema

    bp::child c("/usr/local/bin/node",
                tempFilePath.toStdString(),
                filePath,
                "modify",
                dataId,
                "",
                finalJsxArg,
                env);
    c.wait();
}

void FrontendGenerator::applyDeletion(std::shared_ptr<BaseNode> &node)
{
    // Si la modificación es en un view o custom component
    if (node->getNodeType() == "Section")
        return;

    // Si la modificación es en un Component

    // Determinar en qué archivo se debe insertar el nodo
    std::string filePath = getFilePathForNode(node);

    // Obtener la el dataId
    std::string dataId = boost::uuids::to_string(node->getId());

    QString resourcePath = ":/babel/editor";
    QFile resourceFile(resourcePath);
    if (!resourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmt::print(stderr, "❌ Unable to open resource: {}\n", resourcePath.toStdString());
        return;
    }

    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString tempFilePath = tempPath + "/editor.js";

    QFile tempFile(tempFilePath);
    if (tempFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        tempFile.write(resourceFile.readAll());
        tempFile.close();
    } else {
        fmt::print(stderr, "❌ Unable to write temporary editor.js\n");
        return;
    }

    // Correr el comando externo con Node.js

    namespace bp = boost::process;

    bp::environment env = boost::this_process::environment();
    env["NODE_PATH"] = "/usr/local/lib/node_modules"; // Ajusta según tu sistema

    bp::child c("/usr/local/bin/node",
                tempFilePath.toStdString(),
                filePath,
                "delete",
                dataId,
                "",
                "",
                env);
    c.wait();
}

std::string FrontendGenerator::getFilePathForNode(std::shared_ptr<BaseNode> &node)
{
    // Si el nodo es un Section, determinamos el archivo según el tipo
    if (node->getNodeType() == "Section" && node->getParent()->getNodeType() != "Section"
        && node->getParent()->getNodeType() != "Component") {
        auto section = std::dynamic_pointer_cast<Section>(node);
        // Si el section tiene un path, asumimos que es un view
        if (!section->getPath().empty()) {
            return projectPath + "/frontend/src/views/" + section->getName() + ".tsx";
        } else {
            // De lo contrario, es un custom component
            return projectPath + "/frontend/src/components/" + section->getName() + ".tsx";
        }
    }

    // Si es un Component u otro nodo, recorrer hacia arriba para obtener su Section contenedor
    auto parent = node->getParent();
    if (parent) {
        return getFilePathForNode(parent);
    }

    // Si no se encontró, retorna cadena vacía o lanza un error según convenga
    return "";
}

std::string FrontendGenerator::generateNodeFragment(std::shared_ptr<BaseNode> &node)
{
    nlohmann::json data;
    std::string templatePath;

    // Determinar qué tipo de nodo es y cargar el JSON y plantilla adecuada
    if (node->getNodeType() == "Section") {
        auto section = std::dynamic_pointer_cast<Section>(node);
        if (!section) {
            fmt::print(stderr, "Error: nodo identificado como section pero falla el cast.\n");
            return "";
        }
        data = processSectionToJson(section);
        // Si el section tiene un path, lo consideramos un view; de lo contrario, un custom component.
        if (!section->getPath().empty()) {
            templatePath = ":/inja/frontend/view";
        } else {
            templatePath = ":/inja/frontend/view";
        }
    } else if (node->getNodeType() == "Component") {
        // Si el nodo es un componente
        auto component = std::dynamic_pointer_cast<Component>(node);
        if (!component) {
            fmt::print(stderr, "Error: nodo identificado como component pero falla el cast.\n");
            return "";
        }
        // Aquí podrías tener una función específica para componentes, o reutilizar processSectionToJson si aplica
        data = processComponentToJson(component);
        // templatePath = ":/inja/frontend/component_fragment";
    } else {
        fmt::print(stderr,
                   "generateNodeFragment: Tipo de nodo no soportado para generación de "
                   "fragmento.\n");
        return "";
    }

    std::string templateString;
    // Cargar el template desde recursos
    if (templatePath.empty()) {
        templateString = "{{ render_component(data, \"\") }}";
    } else {
        QFile file(QString::fromStdString(templatePath));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            fmt::print(stderr, "Unable to open template file from resource: {}\n", templatePath);
            return "";
        }
        QTextStream in(&file);
        QString templateContent = in.readAll();
        file.close();

        templateString = templateContent.toStdString();
    }

    try {
        // Renderizar el fragmento usando Inja
        std::string result = env.render(templateString, {{"data", data}});
        return result;
    } catch (const std::exception &e) {
        fmt::print(stderr,
                   "Error rendering fragment for node {}: {}\n",
                   boost::uuids::to_string(node->getId()),
                   e.what());
        return "";
    }
}

void FrontendGenerator::getReferenceForInsertion(std::string &referenceId,
                                                 std::string &position,
                                                 std::shared_ptr<BaseNode> &node)
{
    if (!node->getParent())
        return;

    auto parent = node->getParent();
    auto children = parent->getChildren();

    if (children.empty()) {
        if (auto section = std::dynamic_pointer_cast<Section>(parent)) {
            referenceId = "none";
            position = "inner";
        }
        return;
    }

    auto it = std::find(children.begin(), children.end(), node);
    if (it == children.end()) {
        fmt::print("Error: node {} wasn't found among parent's children.\n",
                   boost::uuids::to_string(node->getId()));
        return;
    }

    size_t index = std::distance(children.begin(), it);

    if (children.size() == 1) {
        // Único hijo, se inserta dentro del padre (opcional)
        referenceId = "none";
        position = "inner";
    } else if (index == 0) {
        // Es el primero, usar el segundo como referencia con "before"
        referenceId = boost::uuids::to_string(children.at(1)->getId());
        position = "before";
    } else {
        // Insertar después del anterior
        referenceId = boost::uuids::to_string(children.at(index - 1)->getId());
        position = "after";
    }
}

// Getters
const std::shared_ptr<BaseNode> &FrontendGenerator::getFrontendRoot() const
{
    return frontendRoot;
}

bool FrontendGenerator::isProgressSaved()
{
    std::vector<NodeOperation> operations = diffTrees(oldRoot, frontendRoot);

    if (operations.empty()) {
        qDebug() << "No changes detected.";
        return true;
    } else {
        qDebug() << "Changes detected.";
        return false;
    }
}
