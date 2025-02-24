#include "frontendgenerator.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
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

std::shared_ptr<BaseNode> parseComponent(const nlohmann::json &componentJson)
{
    // If it's a component
    if (componentJson.contains("type")) {
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
            createdOn = parseDateTime(componentJson["createdOn"].get<std::string>());
        }

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

        return component;
    } else {
        std::string name = componentJson["name"].get<std::string>();
        auto section = std::make_shared<Section>(name);
        return section;
    }
}

std::vector<std::shared_ptr<BaseNode>> parseNestedComponents(const nlohmann::json &nestedJsonArray)
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
        auto customSection = std::make_shared<Section>(custComponentJson["name"].get<std::string>());

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
        // Crear una sección para cada vista
        auto viewSection = std::make_shared<Section>(viewJson["name"].get<std::string>(),
                                                     viewJson["path"].get<std::string>());

        // Parsear los componentes dentro de la vista
        for (const auto &componentJson : viewJson["components"]) {
            auto componentNode = parseComponent(componentJson);

            // Verificar si hay componentes anidados
            if (componentJson.contains("nestedComponents")) {
                auto nestedComponents = parseNestedComponents(componentJson["nestedComponents"]);
                for (const auto &nestedComponent : nestedComponents) {
                    componentNode->addChild(nestedComponent);
                }
            }

            if (componentNode) {
                viewSection->addChild(componentNode);
            }
        }

        // Agregar la vista al nodo principal de views
        viewsNode->addChild(viewSection);
    }

    // printNodeTree(frontendRoot);

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
    return true;
}

bool allowsNestedComponents(ComponentType type)
{
    return type == ComponentType::Form || type == ComponentType::HorizontalLayout
           || type == ComponentType::VerticalLayout || type == ComponentType::ModelLayout;
}

nlohmann::json processSectionToJson(const std::shared_ptr<Section> &section)
{
    nlohmann::json sectionJson;
    sectionJson["name"] = section->getName();
    sectionJson["components"] = nlohmann::json::array();
    if (!section->getPath().empty())
        sectionJson["path"] = section->getPath();

    for (const auto &child : section->getChildren()) {
        auto component = std::dynamic_pointer_cast<Component>(child);
        if (component) {
            nlohmann::json componentJson;
            componentJson["type"] = componentTypeToString(component->getType());
            componentJson["id"] = boost::uuids::to_string(component->getId());
            componentJson["createdOn"] = timePointToString(component->getCreatedOn());
            componentJson["updatedOn"] = timePointToString(component->getUpdatedOn());

            // Agregar props del componente
            nlohmann::json propsJson;
            for (const auto &prop : component->getProps()) {
                propsJson[prop.first] = prop.second;
            }
            componentJson["props"] = propsJson;

            // Manejo de nestedComponents
            if (component->isAllowingItems()) {
                componentJson["nestedComponents"] = nlohmann::json::array();
                for (const auto &nestedComponent : component->getChildren()) {
                    nlohmann::json nestedComponentJson;
                    auto nestedComponentPtr = std::dynamic_pointer_cast<Component>(nestedComponent);
                    nestedComponentJson["type"] = componentTypeToString(nestedComponentPtr->getType());
                    nestedComponentJson["id"] = boost::uuids::to_string(nestedComponentPtr->getId());
                    nestedComponentJson["createdOn"] = timePointToString(
                        nestedComponentPtr->getCreatedOn());
                    nestedComponentJson["updatedOn"] = timePointToString(
                        nestedComponentPtr->getUpdatedOn());

                    // Props de los nestedComponents
                    nlohmann::json nestedPropsJson;
                    for (const auto &nestedProp : nestedComponentPtr->getProps()) {
                        nestedPropsJson[nestedProp.first] = nestedProp.second;
                    }
                    nestedComponentJson["props"] = nestedPropsJson;

                    componentJson["nestedComponents"].push_back(nestedComponentJson);
                }
            }

            sectionJson["components"].push_back(componentJson);

        } else {
            auto subSection = std::dynamic_pointer_cast<Section>(child);
            if (subSection) {
                nlohmann::json subSectionJson;
                subSectionJson["name"] = subSection->getName();
                sectionJson["components"].push_back(subSectionJson);
            }
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

bool FrontendGenerator::updateSchema()
{
    nlohmann::json jsonSchema;

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

void processSection(const std::shared_ptr<Section> &section, nlohmann::json &jsonArray)
{
    for (const auto &child : section->getChildren()) {
        auto component = std::dynamic_pointer_cast<Component>(child);
        if (component) {
            nlohmann::json componentJson;
            componentJson["type"] = componentTypeToString(component->getType());
            componentJson["id"] = boost::uuids::to_string(component->getId());
            componentJson["createdOn"] = timePointToString(component->getCreatedOn());
            componentJson["updatedOn"] = timePointToString(component->getUpdatedOn());

            // Agrega las props si existen
            nlohmann::json propsJson;
            for (const auto &prop : component->getProps()) {
                propsJson[prop.first] = prop.second;
            }
            componentJson["props"] = propsJson;

            // Manejo de nestedComponents para tipos permitidos
            if (component->isAllowingItems()) {
                componentJson["nestedComponents"] = nlohmann::json::array();
                for (const auto &nestedChild : component->getChildren()) {
                    nlohmann::json nestedComponentJson;
                    auto nestedChildPtr = std::dynamic_pointer_cast<Component>(nestedChild);
                    nestedComponentJson["type"] = componentTypeToString(nestedChildPtr->getType());
                    nestedComponentJson["id"] = boost::uuids::to_string(nestedChildPtr->getId());
                    nestedComponentJson["createdOn"] = timePointToString(
                        nestedChildPtr->getCreatedOn());
                    nestedComponentJson["updatedOn"] = timePointToString(
                        nestedChildPtr->getUpdatedOn());

                    // Agregar las props del nestedComponent
                    nlohmann::json nestedPropsJson;
                    for (const auto &nestedProp : nestedChildPtr->getProps()) {
                        nestedPropsJson[nestedProp.first] = nestedProp.second;
                    }
                    nestedComponentJson["props"] = nestedPropsJson;

                    componentJson["nestedComponents"].push_back(nestedComponentJson);
                }
            }

            jsonArray.push_back(componentJson);

        } else {
            auto subSection = std::dynamic_pointer_cast<Section>(child);
            if (subSection) {
                nlohmann::json sectionJson;
                sectionJson["name"] = subSection->getName();

                jsonArray.push_back(sectionJson);
            }
        }
    }
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

bool FrontendGenerator::generateView(const std::string &viewName)
{
    nlohmann::json data;

    // Inserta el nombre del componente en el contexto de Inja
    data["component"] = viewName;
    data["components"] = nlohmann::json::array(); // Asegúrate de inicializar el array

    auto view = findViewByName(viewName);

    if (!view) {
        qDebug() << "View " << viewName << " wasn't found\n";
        return false;
    }

    auto section = std::dynamic_pointer_cast<Section>(view);

    if (!section) {
        qDebug() << "View " << viewName << " couldn't be casted\n";
        return false;
    }

    processSection(section, data["components"]);

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

    // Inserta el nombre del componente en el contexto de Inja
    data["component"] = custComponentName;
    data["components"] = nlohmann::json::array(); // Asegúrate de inicializar el array

    auto custComp = findViewByName(custComponentName);

    if (!custComp) {
        qDebug() << "Custom Component " << custComponentName << " wasn't found\n";
        return false;
    }

    auto section = std::dynamic_pointer_cast<Section>(custComp);

    if (!custComp) {
        qDebug() << "Custom Component " << custComponentName << " couldn't be casted\n";
        return false;
    }

    processSection(section, data["components"]);

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
            if (!generateView(section->getName())) {
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

bool FrontendGenerator::updateFrontendCode()
{
    if (updateSchema()) {
        if (generateFrontendCode())
            return true;
        else
            qDebug() << "Failing in GENERATING FRONTEND CODE";
    } else {
        qDebug() << "Failing in UPDATING SCHEMA";
        return false;
    }
    // return (updateSchema() ? generateFrontendCode() : false);
}

// Getters
const std::shared_ptr<BaseNode> &FrontendGenerator::getFrontendRoot() const
{
    return frontendRoot;
}
