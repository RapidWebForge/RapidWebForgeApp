#include "frontendgenerator.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include "../../models/component-type/componenttype.h"
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

    for (const auto &customComponent : this->custComponents) {
        std::string name = customComponent->getName();
        nlohmann::json jsonRepresentation = processSectionToJson(customComponent);
        RenderCallback::customComponentsCache[name] = jsonRepresentation;
    }

    fmt::print("Custom components cache initialized with {} items.\n",
               RenderCallback::customComponentsCache.size());
}

bool FrontendGenerator::loadSchema()
{
    std::ifstream file(projectPath + "/frontend.json");
    if (!file.is_open()) {
        fmt::print(stderr, "Unable to open JSON file: {}/frontend.json\n", projectPath);

        // Crear frontend.json base si no existe
        nlohmann::json frontendJson;
        frontendJson["routes"] = nlohmann::json::array();
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

    parseJson(jsonSchema); // JSON to routes, views and custom components
    // initializeCustomComponentsCache();
    return true;
}

bool allowsNestedComponents(ComponentType type)
{
    return type == ComponentType::Form || type == ComponentType::HorizontalLayout
           || type == ComponentType::VerticalLayout || type == ComponentType::ModelLayout;
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

std::shared_ptr<Component> FrontendGenerator::parseComponent(const nlohmann::json &componentJson)
{
    boost::uuids::uuid id;

    if (componentJson.contains("id") && componentJson["id"].is_string()) {
        boost::uuids::string_generator gen;
        id = gen(componentJson["id"].get<std::string>());
    }

    std::chrono::system_clock::time_point createdOn, updatedOn;

    if (componentJson.contains("createdOn") && componentJson["createdOn"].is_number()) {
        createdOn = std::chrono::system_clock::from_time_t(
            componentJson["createdOn"].get<std::time_t>());
    }

    if (componentJson.contains("updatedOn") && componentJson["updatedOn"].is_number()) {
        updatedOn = std::chrono::system_clock::from_time_t(
            componentJson["updatedOn"].get<std::time_t>());
    }

    auto component = std::make_shared<Component>(id, createdOn, updatedOn);

    // Parse type
    if (componentJson.contains("type") && componentJson["type"].is_string()) {
        component->setType(stringToComponentType(componentJson["type"].get<std::string>()));

        // Parse nested components if applicable
        if (allowsNestedComponents(component->getType())
            && componentJson.contains("nestedComponents")
            && componentJson["nestedComponents"].is_array()) {
            auto baseNodes = parseNestedComponents(componentJson["nestedComponents"]);

            // Convertir a std::vector<std::shared_ptr<Component>>
            std::vector<std::shared_ptr<BaseNode>> nestedComponents;
            for (const auto &baseNode : baseNodes) {
                auto nestedComponent = std::dynamic_pointer_cast<Component>(baseNode);
                if (nestedComponent) {
                    nestedComponents.push_back(nestedComponent);
                }
            }

            component->setNestedComponents(nestedComponents);
        }
    } else {
        fmt::print(stderr, "Error: 'type' in 'components' must be a string.\n");
    }

    // Parse props
    std::map<std::string, std::string> props;

    if (componentJson.contains("props") && componentJson["props"].is_object()) {
        for (auto it = componentJson["props"].begin(); it != componentJson["props"].end(); ++it) {
            if (it.value().is_string()) {
                props[it.key()] = it.value().get<std::string>();
            } else {
                fmt::print(stderr, "Error: 'props' value for '{}' must be a string.\n", it.key());
            }
        }
    }

    component->setProps(props);

    return component;
}

void FrontendGenerator::parseJson(const nlohmann::json &jsonSchema)
{
    // Parse routes
    for (const auto &routeJson : jsonSchema["routes"]) {
        Route route;
        if (routeJson.contains("component") && routeJson["component"].is_string()) {
            route.setComponent(routeJson["component"].get<std::string>());
        } else {
            fmt::print(stderr, "Error: 'component' in 'routes' must be a string.\n");
            continue;
        }

        if (routeJson.contains("path") && routeJson["path"].is_string()) {
            route.setPath(routeJson["path"].get<std::string>());
        } else {
            fmt::print(stderr, "Error: 'path' in 'routes' must be a string.\n");
            continue;
        }

        routes.push_back(route);
    }

    // Parse views
    for (const auto &viewJson : jsonSchema["views"]) {
        for (auto it = viewJson.begin(); it != viewJson.end(); ++it) {
            auto section = std::make_shared<Section>(it.key());

            // Parse components
            for (const auto &componentJson : it.value()["components"]) {
                auto component = parseComponent(componentJson);
                if (component) {
                    section->addComponent(component);
                }
            }

            views.push_back(section);
        }
    }

    // Parse custom components
    for (const auto &custComponentJson : jsonSchema["custom"]) {
        for (auto it = custComponentJson.begin(); it != custComponentJson.end(); ++it) {
            auto section = std::make_shared<Section>(it.key());

            // Parse components
            for (const auto &componentJson : it.value()["components"]) {
                auto component = parseComponent(componentJson);
                if (component) {
                    section->addComponent(component);
                }
            }

            custComponents.push_back(section);
        }
    }
}

nlohmann::json FrontendGenerator::processSectionToJson(const std::shared_ptr<Section> &section)
{
    nlohmann::json sectionJson;
    sectionJson["components"] = nlohmann::json::array();

    for (const auto &child : section->getComponents()) {
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
                for (const auto &nestedComponent : component->getNestedComponents()) {
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
                nlohmann::json subSectionJson = processSectionToJson(subSection);
                subSectionJson["name"] = subSection->getName();
                sectionJson["components"].push_back(subSectionJson);
            }
        }
    }

    return sectionJson;
}

bool FrontendGenerator::updateSchema()
{
    nlohmann::json jsonSchema;

    // Create the routes
    jsonSchema["routes"] = nlohmann::json::array();
    for (const auto &route : this->routes) {
        nlohmann::json routeJson;
        routeJson["component"] = route.getComponent();
        routeJson["path"] = route.getPath();
        jsonSchema["routes"].push_back(routeJson);
    }

    // Create the views
    jsonSchema["views"] = nlohmann::json::array();
    for (const auto &view : this->views) {
        auto section = std::dynamic_pointer_cast<Section>(view);
        if (section) {
            nlohmann::json viewJson;
            viewJson[section->getName()] = processSectionToJson(section);
            jsonSchema["views"].push_back(viewJson);
        }
    }

    // Create the custom components
    jsonSchema["custom"] = nlohmann::json::array();
    for (const auto &customComponent : this->custComponents) {
        auto section = std::dynamic_pointer_cast<Section>(customComponent);
        if (section) {
            nlohmann::json customComponentJson;
            customComponentJson[section->getName()] = processSectionToJson(section);
            jsonSchema["custom"].push_back(customComponentJson);
        }
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

void FrontendGenerator::processSection(const std::shared_ptr<Section> &section,
                                       nlohmann::json &jsonArray)
{
    for (const auto &child : section->getComponents()) {
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
                for (const auto &nestedChild : component->getNestedComponents()) {
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
                sectionJson["components"] = nlohmann::json::array();

                // Procesar subsección recursivamente
                processSection(subSection, sectionJson["components"]);

                jsonArray.push_back(sectionJson);
            }
        }
    }
}

bool FrontendGenerator::generateView(const std::string &viewName)
{
    nlohmann::json data;

    // Inserta el nombre del componente en el contexto de Inja
    data["component"] = viewName;

    // Buscar la vista en views
    auto it = std::find_if(views.begin(),
                           views.end(),
                           [&viewName](const std::shared_ptr<Section> &view) {
                               return view->getName() == viewName;
                           });

    data["components"] = nlohmann::json::array(); // Asegúrate de inicializar el array

    // Si la vista existe
    if (it != views.end()) {
        auto section = std::dynamic_pointer_cast<Section>(*it); // Convertir BaseNode a Section
        if (section) {
            // Renderizar al json
            processSection(section, data["components"]);
        }
    }

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
        fmt::print(stderr, "Error generating component base for {}: {}\n", viewName, e.what());
        return false;
    }

    // fmt::print("Component base generated successfully for {}\n", viewName);
    return true;
}

bool FrontendGenerator::generateCustomComponent(const std::string &custComponentName)
{
    nlohmann::json data;

    // Inserta el nombre del componente en el contexto de Inja
    data["component"] = custComponentName;

    // Buscar la vista en custComponents
    auto it = std::find_if(custComponents.begin(),
                           custComponents.end(),
                           [&custComponentName](const std::shared_ptr<Section> &cc) {
                               return cc->getName() == custComponentName;
                           });

    data["components"] = nlohmann::json::array(); // Asegúrate de inicializar el array

    // Si la vista existe
    if (it != custComponents.end()) {
        auto section = std::dynamic_pointer_cast<Section>(*it); // Convertir BaseNode a Section
        if (section) {
            // Renderizar al json
            processSection(section, data["components"]);
        }
    }

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
        fmt::print(stderr, "Error custom component {}: {}\n", custComponentName, e.what());
        return false;
    }

    fmt::print("Custom component generated successfully for {}\n", custComponentName);
    return true;
}

bool FrontendGenerator::generateApp()
{
    nlohmann::json data;

    // Insert the routes into the JSON context for Inja
    data["routes"] = nlohmann::json::array();

    // Create views in base of the routes json
    for (const auto &route : this->routes) {
        nlohmann::json routeJson;
        routeJson["component"] = route.getComponent();
        routeJson["path"] = route.getPath();
        data["routes"].push_back(routeJson);

        if (!generateView(route.getComponent())) {
            fmt::print(stderr, "Failed to generate component base for {}\n", route.getComponent());
            return false;
        }
    }

    for (const auto &custComponent : this->custComponents) {
        if (!generateCustomComponent(custComponent->getName())) {
            fmt::print(stderr, "Failed to custom component {}\n", custComponent->getName());
            return false;
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

bool FrontendGenerator::generateFrontendCode()
{
    initializeCustomComponentsCache();
    return generateApp();
}

bool FrontendGenerator::updateFrontendCode()
{
    return (updateSchema() ? generateFrontendCode() : false);
}

// Getters
const std::vector<Route> &FrontendGenerator::getRoutes() const
{
    return routes;
}

const std::vector<std::shared_ptr<Section>> &FrontendGenerator::getViews() const
{
    return views;
}

const std::vector<std::shared_ptr<Section>> &FrontendGenerator::getCustomComponents() const
{
    return custComponents;
}

// Setters
void FrontendGenerator::setRoutes(const std::vector<Route> &routes)
{
    this->routes = routes;
}

void FrontendGenerator::setViews(const std::vector<std::shared_ptr<Section>> &views)
{
    this->views = views;
}

void FrontendGenerator::setCustomComponents(
    const std::vector<std::shared_ptr<Section>> &custComponents)
{
    this->custComponents = custComponents;
}
