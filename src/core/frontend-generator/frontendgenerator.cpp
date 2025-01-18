#include "frontendgenerator.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include "../../models/component-type/componenttype.h"
#include "../../utils/file/fileutiils.h"
#include "../../utils/render_callback/rendercallback.h"
#include <fmt/core.h>
#include <fstream>

FrontendGenerator::FrontendGenerator(const std::string &projectPath)
    : projectPath(projectPath)
{
    try {
        env.add_callback("render_component", [this](inja::Arguments &args) -> std::string {
            return RenderCallback::renderComponentCallback(this->env, args);
        });
        env.add_callback("render_services_imports", 1, [this](inja::Arguments &args) -> std::string {
            return RenderCallback::renderServiceImportsCallback(this->env, args);
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

bool FrontendGenerator::loadSchema()
{
    std::ifstream file(projectPath + "/frontend.json");
    if (!file.is_open()) {
        fmt::print(stderr, "Unable to open JSON file: {}/frontend.json\n", projectPath);

        // Crear frontend.json base si no existe
        nlohmann::json frontendJson;
        frontendJson["routes"] = nlohmann::json::array();
        frontendJson["views"] = nlohmann::json::array();
        // Custom components
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

    parseJson(jsonSchema); // Convertir el JSON a rutas y vistas
    return true;
}

bool allowsNestedComponents(ComponentType type)
{
    return type == ComponentType::Form || type == ComponentType::HorizontalLayout
           || type == ComponentType::VerticalLayout || type == ComponentType::ModelLayout;
}

std::vector<Component> FrontendGenerator::parseNestedComponents(const nlohmann::json &nestedJsonArray)
{
    std::vector<Component> nestedComponents;
    for (const auto &nestedJson : nestedJsonArray) {
        Component nestedComponent = parseComponent(nestedJson);

        nestedComponents.push_back(nestedComponent);
    }
    return nestedComponents;
}

Component FrontendGenerator::parseComponent(const nlohmann::json &componentJson)
{
    Component component;

    // Parse type
    if (componentJson.contains("type") && componentJson["type"].is_string()) {
        component.setType(stringToComponentType(componentJson["type"].get<std::string>()));

        // Parse nested components if applicable
        if (allowsNestedComponents(component.getType())
            && componentJson.contains("nestedComponents")
            && componentJson["nestedComponents"].is_array()) {
            component.setNestedComponents(parseNestedComponents(componentJson["nestedComponents"]));
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

    component.setProps(props);

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
            Section view;
            view.setName(it.key());

            // Parse components
            std::vector<Component> components;
            for (const auto &componentJson : it.value()["components"]) {
                Component component = parseComponent(componentJson);

                components.push_back(component);
            }

            view.setComponents(components);
            views.push_back(view);
        }
    }

    // Parse custom components
    for (const auto &custComponentJson : jsonSchema["custom"]) {
        for (auto it = custComponentJson.begin(); it != custComponentJson.end(); ++it) {
            Section customComponent;
            customComponent.setName(it.key());

            // Parse components
            std::vector<Component> components;
            for (const auto &componentJson : it.value()["components"]) {
                Component component = parseComponent(componentJson);

                components.push_back(component);
            }

            customComponent.setComponents(components);
            custComponents.push_back(customComponent);
        }
    }
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
        nlohmann::json viewJson;
        viewJson[view.getName()]["components"] = nlohmann::json::array();

        for (const auto &component : view.getComponents()) {
            nlohmann::json componentJson;

            // Convertir ComponentType a cadena antes de asignarlo al JSON
            std::string typeStr = componentTypeToString(component.getType());
            componentJson["type"] = typeStr;

            // Agregar las props
            nlohmann::json propsJson;
            // std::cout << "  Component Type: " << typeStr << std::endl;
            for (const auto &prop : component.getProps()) {
                propsJson[prop.first] = prop.second;
                // std::cout << "    Prop: " << prop.first << " = " << prop.second << std::endl;
            }
            componentJson["props"] = propsJson;

            // Si el componente permite nestedComponents, agregarlo al JSON
            if (component.isAllowingItems()) {
                componentJson["nestedComponents"] = nlohmann::json::array();
                for (const auto &nestedComponent : component.getNestedComponents()) {
                    nlohmann::json nestedComponentJson;

                    // Convertir el tipo de componente anidado
                    std::string nestedTypeStr = componentTypeToString(nestedComponent.getType());
                    nestedComponentJson["type"] = nestedTypeStr;

                    // Agregar las props del componente anidado
                    nlohmann::json nestedPropsJson;
                    for (const auto &nestedProp : nestedComponent.getProps()) {
                        nestedPropsJson[nestedProp.first] = nestedProp.second;
                    }
                    nestedComponentJson["props"] = nestedPropsJson;

                    componentJson["nestedComponents"].push_back(nestedComponentJson);
                }
            }

            viewJson[view.getName()]["components"].push_back(componentJson);
        }

        jsonSchema["views"].push_back(viewJson);
    }

    // Create the custom components
    jsonSchema["custom"] = nlohmann::json::array();
    for (const auto &customComponent : this->custComponents) {
        nlohmann::json customComponentJson;
        customComponentJson[customComponent.getName()]["components"] = nlohmann::json::array();

        for (const auto &component : customComponent.getComponents()) {
            nlohmann::json componentJson;

            // Convertir ComponentType a cadena antes de asignarlo al JSON
            std::string typeStr = componentTypeToString(component.getType());
            componentJson["type"] = typeStr;

            // Agregar las props
            nlohmann::json propsJson;
            // std::cout << "  Component Type: " << typeStr << std::endl;
            for (const auto &prop : component.getProps()) {
                propsJson[prop.first] = prop.second;
                // std::cout << "    Prop: " << prop.first << " = " << prop.second << std::endl;
            }
            componentJson["props"] = propsJson;

            // Si el componente permite nestedComponents, agregarlo al JSON
            if (component.isAllowingItems()) {
                componentJson["nestedComponents"] = nlohmann::json::array();
                for (const auto &nestedComponent : component.getNestedComponents()) {
                    nlohmann::json nestedComponentJson;

                    // Convertir el tipo de componente anidado
                    std::string nestedTypeStr = componentTypeToString(nestedComponent.getType());
                    nestedComponentJson["type"] = nestedTypeStr;

                    // Agregar las props del componente anidado
                    nlohmann::json nestedPropsJson;
                    for (const auto &nestedProp : nestedComponent.getProps()) {
                        nestedPropsJson[nestedProp.first] = nestedProp.second;
                    }
                    nestedComponentJson["props"] = nestedPropsJson;

                    componentJson["nestedComponents"].push_back(nestedComponentJson);
                }
            }

            customComponentJson[customComponent.getName()]["components"].push_back(componentJson);
        }

        jsonSchema["custom"].push_back(customComponentJson);
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

bool FrontendGenerator::generateView(const std::string &viewName)
{
    nlohmann::json data;

    // Inserta el nombre del componente en el contexto de Inja
    data["component"] = viewName;

    // Buscar la vista en views
    auto it = std::find_if(views.begin(), views.end(), [&viewName](const Section &view) {
        return view.getName() == viewName;
    });

    data["components"] = nlohmann::json::array(); // Asegúrate de inicializar el array

    // Si la vista ya existe, agrega sus componentes al contexto
    if (it != views.end()) {
        for (const Component &component : it->getComponents()) {
            nlohmann::json componentJson;
            componentJson["type"] = componentTypeToString(component.getType());

            // Agrega las props si existen
            nlohmann::json propsJson;
            for (const auto &prop : component.getProps()) {
                propsJson[prop.first] = prop.second;
            }
            componentJson["props"] = propsJson;

            // Manejo de nestedComponents para tipos permitidos
            if (component.isAllowingItems()) {
                componentJson["nestedComponents"] = nlohmann::json::array();
                for (const auto &nestedComponent : component.getNestedComponents()) {
                    nlohmann::json nestedComponentJson;
                    nestedComponentJson["type"] = componentTypeToString(nestedComponent.getType());

                    // Agregar las props del nestedComponent
                    nlohmann::json nestedPropsJson;
                    for (const auto &nestedProp : nestedComponent.getProps()) {
                        nestedPropsJson[nestedProp.first] = nestedProp.second;
                    }
                    nestedComponentJson["props"] = nestedPropsJson;

                    componentJson["nestedComponents"].push_back(nestedComponentJson);
                }
            }

            data["components"].push_back(componentJson);
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

        // bool viewExists = false;

        // std::string viewFilePath = projectPath + "frontend/src/views/" + route.getComponent()
        //                            + ".tsx";

        // Verificar si el archivo ya existe
        // std::ifstream fileCheck(viewFilePath);
        // if (fileCheck.is_open()) {
        //     // El archivo ya existe, no es necesario generarlo de nuevo
        //     fileCheck.close();
        // } else {
        //     viewExists = true;
        // }

        // if (!viewExists) {
        if (!generateView(route.getComponent())) {
            fmt::print(stderr, "Failed to generate component base for {}\n", route.getComponent());
            return false;
        }
        // }
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
    return !generateApp();
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

const std::vector<Section> &FrontendGenerator::getViews() const
{
    return views;
}

std::vector<Section> &FrontendGenerator::getViews()
{
    return views;
}

const std::vector<Section> &FrontendGenerator::getCustomComponents() const
{
    return custComponents;
}

std::vector<Section> &FrontendGenerator::getCustomComponents()
{
    return custComponents;
}

// Setters
void FrontendGenerator::setRoutes(const std::vector<Route> &routes)
{
    this->routes = routes;
}

void FrontendGenerator::setViews(const std::vector<Section> &views)
{
    this->views = views;
}

void FrontendGenerator::setCustomComponents(const std::vector<Section> &custComponents)
{
    this->custComponents = custComponents;
}
