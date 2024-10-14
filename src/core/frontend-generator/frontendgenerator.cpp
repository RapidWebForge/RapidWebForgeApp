#include "frontendgenerator.h"

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
