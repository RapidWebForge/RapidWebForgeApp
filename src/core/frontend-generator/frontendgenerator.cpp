#include "frontendgenerator.h"
#include <QFile>
#include <QTextStream>
#include "../../utils/file/fileutiils.h"
#include <fmt/core.h>
#include <fstream>
#include <inja/inja.hpp>

FrontendGenerator::FrontendGenerator(const std::string &projectPath)
    : projectPath(projectPath)
{}

bool FrontendGenerator::loadSchema()
{
    std::ifstream file(projectPath + "/frontend.json");
    if (!file.is_open()) {
        fmt::print(stderr, "Unable to open JSON file: {}/frontend.json\n", projectPath);

        // Crear frontend.json base si no existe
        nlohmann::json frontendJson;
        frontendJson["routes"] = nlohmann::json::array();
        frontendJson["views"] = nlohmann::json::array();

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

    parseJson(jsonSchema); // Convertir el JSON a rutas y vistas
    return true;
}

void FrontendGenerator::parseJson(const nlohmann::json &jsonSchema)
{
    // Parse routes
    for (const auto &routeJson : jsonSchema["routes"]) {
        Route route;
        route.setComponent(routeJson["component"].get<std::string>());
        route.setPath(routeJson["path"].get<std::string>());
        routes.push_back(route);
    }

    // Parse views
    for (const auto &viewJson : jsonSchema["views"]) {
        for (auto it = viewJson.begin(); it != viewJson.end(); ++it) {
            View view;
            view.setName(it.key());

            // Parse components
            std::vector<Component> components;
            for (const auto &componentJson : it.value()["components"]) {
                Component component;
                component.setType(componentJson["type"].get<std::string>());

                // Parse props
                std::map<std::string, std::string> props;
                for (auto propIt = componentJson["props"].begin();
                     propIt != componentJson["props"].end();
                     ++propIt) {
                    props[propIt.key()] = propIt.value().get<std::string>();
                }
                component.setProps(props);

                components.push_back(component);
            }
            view.setComponents(components);
            views.push_back(view);
        }
    }
}

bool FrontendGenerator::updateSchema()
{
    nlohmann::json jsonSchema;

    // Crear las rutas
    jsonSchema["routes"] = nlohmann::json::array();
    for (const auto &route : routes) {
        nlohmann::json routeJson;
        routeJson["component"] = route.getComponent();
        routeJson["path"] = route.getPath();
        jsonSchema["routes"].push_back(routeJson);
    }

    // Crear las vistas
    jsonSchema["views"] = nlohmann::json::array();
    for (const auto &view : views) {
        nlohmann::json viewJson;
        viewJson[view.getName()]["components"] = nlohmann::json::array();

        for (const auto &component : view.getComponents()) {
            nlohmann::json componentJson;
            componentJson["type"] = component.getType();

            // Agregar las props
            nlohmann::json propsJson;
            for (const auto &prop : component.getProps()) {
                propsJson[prop.first] = prop.second;
            }
            componentJson["props"] = propsJson;

            viewJson[view.getName()]["components"].push_back(componentJson);
        }

        jsonSchema["views"].push_back(viewJson);
    }

    // Guardar en el archivo
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

bool FrontendGenerator::updateFrontendJson(const std::string &componentName)
{
    // Load the existing frontend.json
    std::ifstream jsonFile(projectPath + "/frontend.json");
    nlohmann::json frontendJson;

    if (!jsonFile.is_open()) {
        fmt::print(stderr, "Failed to open frontend.json for updating.\n");
        return false;
    }

    try {
        jsonFile >> frontendJson;
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error parsing frontend.json: {}\n", e.what());
        return false;
    }
    jsonFile.close();

    // Check if the component already exists in the views array
    bool componentExists = false;
    for (auto &view : frontendJson["views"]) {
        if (view.contains(componentName)) {
            componentExists = true;
            break;
        }
    }

    // If the component does not exist, add it to the JSON
    if (!componentExists) {
        nlohmann::json newView;
        nlohmann::json newComponent;
        newComponent["type"] = "H1";
        newComponent["props"]["text"] = componentName + " View";

        newView[componentName]["components"].push_back(newComponent);
        frontendJson["views"].push_back(newView);

        // Update the routes if necessary
        nlohmann::json newRoute;
        newRoute["path"] = "/" + componentName;
        newRoute["component"] = componentName;
        frontendJson["routes"].push_back(newRoute);
    }

    // Save the updated frontend.json
    std::ofstream updatedJsonFile(projectPath + "/frontend.json");
    if (!updatedJsonFile.is_open()) {
        fmt::print(stderr, "Failed to save updated frontend.json.\n");
        return false;
    }

    updatedJsonFile << frontendJson.dump(2);
    updatedJsonFile.close();

    return true;
}

bool FrontendGenerator::generateComponentBase(const std::string &componentName)
{
    inja::Environment env;
    nlohmann::json data;

    // Insert the component name into the JSON context
    data["component"] = componentName;

    std::string templatePath = ":/inja/frontend/view";

    QFile file(QString::fromStdString(templatePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmt::print(stderr, "Unable to open template file from resource: {}\n", templatePath);
        return false;
    }

    QTextStream in(&file);
    QString templateContent = in.readAll();
    file.close();

    std::string templateString = templateContent.toStdString();

    std::string outputPath = projectPath + "/frontend/src/views/" + componentName + ".tsx";

    try {
        std::string result = env.render(templateString, data);
        FileUtils::writeFile(outputPath, result);

        updateFrontendJson(componentName);
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error generating component base for {}: {}\n", componentName, e.what());
        return false;
    }

    fmt::print("Component base generated successfully for {}\n", componentName);
    return true;
}

bool FrontendGenerator::generateMain()
{
    inja::Environment env;
    nlohmann::json data;

    // Insert the routes into the JSON context for Inja
    data["routes"] = nlohmann::json::array();
    for (const auto &route : routes) {
        nlohmann::json routeJson;
        routeJson["component"] = route.getComponent();
        routeJson["path"] = route.getPath();
        data["routes"].push_back(routeJson);

        bool componentExists = false;
        for (const auto &view : views) {
            if (view.getName() == route.getComponent()) {
                componentExists = true;
                break;
            }
        }

        if (!componentExists) {
            generateComponentBase(route.getComponent());

            View view(route.getComponent());
            views.push_back(view);
        }
    }

    std::string templatePath = ":/inja/frontend/main";

    QFile file(QString::fromStdString(templatePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmt::print(stderr, "Unable to open template file from resource: {}\n", templatePath);
        return false;
    }

    QTextStream in(&file);
    QString templateContent = in.readAll();
    file.close();

    std::string templateString = templateContent.toStdString();

    std::string outputPath = projectPath + "/frontend/src/main.tsx";

    try {
        std::string result = env.render(templateString, data);
        FileUtils::writeFile(outputPath, result);
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error generating main.tsx: {}\n", e.what());
        return false;
    }

    return true;
}

bool FrontendGenerator::generateFrontendCode()
{
    if (!generateMain())
        return false;

    return true;
}

bool FrontendGenerator::updateFrontendCode()
{
    return updateSchema() ? generateFrontendCode() : false;
}

// Getters
const std::vector<Route> &FrontendGenerator::getRoutes() const
{
    return routes;
}

std::vector<Route> &FrontendGenerator::getRoutes()
{
    return routes;
}

const std::vector<View> &FrontendGenerator::getViews() const
{
    return views;
}

std::vector<View> &FrontendGenerator::getViews()
{
    return views;
}

// Setters
void FrontendGenerator::setRoutes(const std::vector<Route> &routes)
{
    this->routes = routes;
}

void FrontendGenerator::setViews(const std::vector<View> &views)
{
    this->views = views;
}
