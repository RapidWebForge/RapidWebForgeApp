#include "frontendgenerator.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QList>
#include <QProcess>
#include <QStandardPaths>
#include <QTextStream>
#include "../../core/configuration-manager/configurationmanager.h"
#include "../../models/component-type/componenttype.h"
#include "../../models/generic-node/genericnode.h"
#include "../../models/time-chrono/timechrono.h"
#include "../../utils/file/fileutiils.h"
#include "../../utils/render_callback/rendercallback.h"
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
    auto customComponentsNode = getChildByType(frontendRoot, "CustomComponents");

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

    return nullptr;
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
    initializeCustomComponentsCache();

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

// Updating

bool FrontendGenerator::updateSchema()
{
    nlohmann::json jsonSchema;

    auto viewsNode = getChildByType(frontendRoot, "Views");

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

    auto customComponentsNode = getChildByType(frontendRoot, "CustomComponents");

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

    jsonFile << jsonSchema.dump(2);
    jsonFile.close();

    return true;
}

std::shared_ptr<Section> FrontendGenerator::findViewByName(const std::string &viewName)
{
    // Encontrar el nodo "Views" en el AST
    auto viewsNode = getChildByType(frontendRoot, "Views");
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
    auto custCompNode = getChildByType(frontendRoot, "CustomComponents");
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

    // Usar std::find_if para buscar el custom component por nombre
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

    return nullptr; // No se encontró el custom component
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

    auto viewsNode = getChildByType(frontendRoot, "Views");

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

    auto customComponentsNode = getChildByType(frontendRoot, "CustomComponents");

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

    std::string type = newNode->getNodeType();
    // Si tienen el mismo ID y difieren en contenido, se marca como modificación.

    if (oldNode->getId() == newNode->getId() && oldNode->isDifferentFrom(newNode)
        && type != "FrontendRoot" && type != "Views" && type != "CustomComponents") {
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
        // NO contar los custom components `importados` que no tengan el custom component
        // en el nodo `CustomComponents` porque seria sumar operaciones Delete innecesarias
        // El script de Babel ya elimina las importaciones
        auto subSection = std::dynamic_pointer_cast<Section>(pair.second);
        if (subSection && subSection->getPath().empty()
            && RenderCallback::customComponentsCache.find(subSection->getName())
                   == RenderCallback::customComponentsCache.end()) {
            continue;
        }
        ops.push_back(NodeOperation(OperationType::Delete, pair.second));
    }

    return ops;
}

std::shared_ptr<BaseNode> FrontendGenerator::getChildByType(const std::shared_ptr<BaseNode> &root,
                                                            const std::string &type)
{
    for (const auto &child : root->getChildren()) {
        if (child->getNodeType() == type)
            return child;
    }
    return nullptr;
}

bool FrontendGenerator::updateFrontendCode()
{
    auto oldViews = getChildByType(oldRoot, "Views");
    auto newViews = getChildByType(frontendRoot, "Views");
    auto oldCustom = getChildByType(oldRoot, "CustomComponents");
    auto newCustom = getChildByType(frontendRoot, "CustomComponents");

    std::vector<NodeOperation> operations = diffTrees(oldViews, newViews);
    auto customOps = diffTrees(oldCustom, newCustom);
    operations.insert(operations.end(), customOps.begin(), customOps.end());

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

    if (!updateSchema()) {
        qDebug() << "Error on Updating Schema";
        return false;
    }

    // Actualizar el oldRoot para futuras comparaciones
    oldRoot = cloneNode(frontendRoot);
    return true;
}

void FrontendGenerator::runEditorScript(const std::vector<std::string> stdArgs)
{
    // 1. Volcar el recurso interno a un archivo temporal
    const QString resourcePath = ":/babel/editorFrontend";
    QFile resourceFile(resourcePath);
    if (!resourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmt::print(stderr, "❌ Unable to open resource: {}\n", resourcePath.toStdString());
        return;
    }

    const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    const QString tempFilePath = tempDir + "/editorFront.js";
    QFile tempFile(tempFilePath);
    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        fmt::print(stderr, "❌ Unable to write temporary editorFront.js\n");
        return;
    }
    tempFile.write(resourceFile.readAll());
    tempFile.close();

    // 2. Preparar QProcess
    ConfigurationManager configurationManager;
    const QString bunPath = QString::fromStdString(
        configurationManager.getConfiguration().getBunPath());
    const QString frontendDir = QDir::toNativeSeparators(QString::fromStdString(projectPath)
                                                         + "/frontend");

    // 3. Construir lista de argumentos
    QStringList qArgs;
    qArgs << "run"
          << "edit"
          << "--" << tempFilePath;
    for (const auto &s : stdArgs) {
        qArgs << QString::fromStdString(s);
    }

    // 4. Configurar y lanzar el proceso, heredando stdout/stderr
    QProcess proc;
    proc.setProgram(bunPath);
    proc.setArguments(qArgs);
    proc.setWorkingDirectory(frontendDir);
    proc.setProcessChannelMode(QProcess::ForwardedChannels);

    proc.start();
    if (!proc.waitForFinished(-1)) {
        qWarning() << "El proceso no terminó correctamente.";
    } else {
        qDebug() << "Proceso terminado con código:" << proc.exitCode();
    }
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
            if (generateView(section->getName())) {
                std::string filePath = projectPath + "/frontend/src/App.tsx";

                std::vector<std::string> args = {filePath,
                                                 "create",
                                                 section->getName(),
                                                 section->getPath()};

                // Run script
                runEditorScript(args);
                return;
            }
        }
    } else {
        // Determinar en qué archivo se debe insertar el nodo
        std::string filePath = getFilePathForNode(node);

        // Obtener la el dataId de referencia y la posición correspondiente de este id
        std::string referenceId, position;
        getReferenceForInsertion(referenceId, position, node);

        // Generar el fragmento de código usando la plantilla Inja
        std::string jsxFragment = generateNodeFragment(node);
        std::string payloadJson = nlohmann::json(jsxFragment).dump();

        // Escribir el payload a archivo temporal
        QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        QString payloadPath = tempPath + "/payload.json";
        QFile payloadFile(payloadPath);
        if (payloadFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream out(&payloadFile);
            out << QString::fromStdString(payloadJson);
            payloadFile.close();
        } else {
            fmt::print(stderr, "❌ Unable to write temporary payload.json\n");
            return;
        }

        std::vector<std::string> args = {filePath,
                                         "insert",
                                         referenceId,
                                         position,
                                         payloadPath.toStdString()};

        // Run script
        runEditorScript(args);
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

    // Obtener la el dataId
    std::string dataId = boost::uuids::to_string(node->getId());

    // Generar el fragmento de código usando la plantilla Inja
    std::string jsxFragment = generateNodeFragment(node);
    std::string payloadJson = nlohmann::json(jsxFragment).dump();

    // Escribir el payload a archivo temporal
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString payloadPath = tempPath + "/payload.json";
    QFile payloadFile(payloadPath);
    if (payloadFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QTextStream out(&payloadFile);
        out << QString::fromStdString(payloadJson);
        payloadFile.close();
    } else {
        fmt::print(stderr, "❌ Unable to write temporary payload.json\n");
        return;
    }

    std::vector<std::string> args = {filePath, "modify", dataId, "", payloadPath.toStdString()};

    // Run script
    runEditorScript(args);
}

void FrontendGenerator::applyDeletion(std::shared_ptr<BaseNode> &node)
{
    // Si la modificación es un section
    // Y si es un view o un custom component
    if (node->getNodeType() == "Section"
        && (node->getParent()->getNodeType() == "Views"
            || node->getParent()->getNodeType() == "CustomComponents")) {
        std::string sectionName = std::dynamic_pointer_cast<Section>(node)->getName();

        QString folder = node->getParent()->getNodeType() == "Views" ? "views/" : "components/";
        QString filePath = QDir(QString::fromStdString(projectPath))
                               .filePath("frontend/src/" + folder
                                         + QString::fromStdString(sectionName) + ".tsx");

        FileUtils::deleteFile(filePath);
        applyRefactorForDeletedSection(sectionName,
                                       node->getParent()->getNodeType() == "Views"
                                           ? "View"
                                           : "CustomComponent");
        return;
    }

    // Si la modificación es en un Component

    // Determinar en qué archivo se debe eliminar el nodo
    std::string filePath = getFilePathForNode(node);

    // Obtener la el dataId
    std::string dataId = boost::uuids::to_string(node->getId());

    std::vector<std::string> args = {filePath, "delete", dataId, "", ""};

    // Run script
    runEditorScript(args);
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
    nlohmann::json data, parentProps = nlohmann::json::object();
    std::string templateString = "{{ render_component(data, parentProps) }}";

    // Determinar qué tipo de nodo es y cargar el JSON y plantilla adecuada
    if (node->getNodeType() == "Section") {
        auto section = std::dynamic_pointer_cast<Section>(node);
        if (!section) {
            fmt::print(stderr, "Error: nodo identificado como section pero falla el cast.\n");
            return "";
        }
        data = processSectionToJson(section);
    } else if (node->getNodeType() == "Component") {
        // Si el nodo es un componente
        auto component = std::dynamic_pointer_cast<Component>(node);
        if (!component) {
            fmt::print(stderr, "Error: nodo identificado como component pero falla el cast.\n");
            return "";
        }
        data = processComponentToJson(component);

        if ((componentTypeToString(component->getType()) == "Input") && component->getParent()) {
            auto parent = std::dynamic_pointer_cast<Component>(component->getParent());
            nlohmann::json parentJson = processComponentToJson(parent);
            if (parentJson.contains("props") && parentJson["props"]["model"] != ""
                && parentJson["props"]["method"] != "")
                parentProps = parentJson["props"];
        }
    } else {
        fmt::print(stderr,
                   "generateNodeFragment: Tipo de nodo no soportado para generación de "
                   "fragmento.\n");
        return "";
    }

    try {
        // Renderizar el fragmento usando Inja
        std::string result = env.render(templateString,
                                        {{"data", data}, {"parentProps", parentProps}});
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
        // referenceId = "none";
        referenceId = boost::uuids::to_string(parent->getId());
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

void FrontendGenerator::applyRefactorForDeletedSection(const std::string &sectionName,
                                                       const std::string &sectionType)
{
    namespace fs = std::filesystem;

    std::vector<std::string> filesToCheck;

    // Siempre revisar App.tsx si es una vista
    if (sectionType == "View") {
        filesToCheck.push_back(projectPath + "/frontend/src/App.tsx");
    } else {
        // Revisar todos los archivos de views y components
        std::vector<std::string> folders = {
            projectPath + "/frontend/src/views",
            projectPath + "/frontend/src/components",
        };

        for (const auto &folder : folders) {
            for (const auto &entry : fs::recursive_directory_iterator(folder)) {
                if (entry.is_regular_file() && entry.path().extension() == ".tsx") {
                    filesToCheck.push_back(entry.path().string());
                }
            }
        }
    }

    // Ejecutar refactor-delete en todos los archivos afectados
    for (const auto &filePath : filesToCheck) {
        std::vector<std::string> args = {filePath, "refactor-delete", sectionName, sectionType};

        // Run script
        runEditorScript(args);
    }
}

// Getters
const std::shared_ptr<BaseNode> &FrontendGenerator::getFrontendRoot() const
{
    return frontendRoot;
}

bool FrontendGenerator::isProgressSaved()
{
    auto oldViews = getChildByType(oldRoot, "Views");
    auto newViews = getChildByType(frontendRoot, "Views");
    auto oldCustom = getChildByType(oldRoot, "CustomComponents");
    auto newCustom = getChildByType(frontendRoot, "CustomComponents");

    std::vector<NodeOperation> operations = diffTrees(oldViews, newViews);
    auto customOps = diffTrees(oldCustom, newCustom);
    operations.insert(operations.end(), customOps.begin(), customOps.end());

    if (operations.empty()) {
        qDebug() << "No changes detected.";
        return true;
    } else {
        qDebug() << "Changes detected.";
        return false;
    }
}
