#include "frontendgenerator.h"
#include <QFile>
#include <QTextStream>
#include "../../models/component-type/componenttype.h"
#include "../../utils/file/fileutiils.h"
#include <fmt/core.h>
#include <fstream>

FrontendGenerator::FrontendGenerator(const std::string &projectPath)
    : projectPath(projectPath)
{
    try {
        env.add_callback("render_component", 1, [this](inja::Arguments &args) -> std::string {
            // Verificar si el argumento es válido
            if (args.empty() || !args[0]->is_object()) {
                fmt::print(stderr, "Invalid argument passed to render_component.\n");
                return "<!-- Invalid argument -->";
            }

            const nlohmann::json &componentJson = *args[0];

            // Verificar si el JSON contiene 'type' y si es un string
            std::string type;
            if (componentJson.contains("type") && componentJson["type"].is_string()) {
                type = componentJson["type"];
            } else {
                fmt::print(stderr,
                           "Unsupported component type format or missing: {}\n",
                           componentJson.dump());
                return "<!-- Unsupported component type -->";
            }

            // Verificar si 'props' es un objeto
            const auto &props = componentJson["props"];
            if (!props.is_object()) {
                fmt::print(stderr, "Invalid props format: must be an object.\n");
                return "<!-- Invalid props format -->";
            }

            std::string output;
            // Renderizar componentes según su tipo
            if (type == "Header H1") {
                output = "<h1>" + props.value("text", "Default Header") + "</h1>";
            } else if (type == "Header H2") {
                output = "<h2>" + props.value("text", "Default Header 2") + "</h2>";
            } else if (type == "Paragraph") {
                output = "<p>" + props.value("text", "Default Paragraph") + "</p>";
            } else if (type == "Input") {
                output = "<input placeholder='" + props.value("placeholder", "Default Input")
                         + "' />";
            } else if (type == "Text Area") {
                output = "<textarea placeholder='" + props.value("placeholder", "Default TextArea")
                         + "'></textarea>";
            } else if (type == "Horizontal Layout" || type == "Vertical Layout") {
                std::string layoutClass = (type == "Horizontal Layout") ? "flex flex-row"
                                                                        : "flex flex-col";
                output = "<div className='" + layoutClass + "'>";

                // Renderizar componentes anidados recursivamente
                if (componentJson.contains("nestedComponents")
                    && componentJson["nestedComponents"].is_array()) {
                    for (const auto &nestedComponent : componentJson["nestedComponents"]) {
                        // Renderizar el componente anidado usando render_component
                        try {
                            output += env.render("{{ render_component(nestedComponent) }}",
                                                 nestedComponent);
                        } catch (const std::exception &e) {
                            fmt::print(stderr, "Error rendering nested component: {}\n", e.what());
                            output += "<!-- Error rendering nested component -->";
                        }
                    }
                } else {
                    fmt::print(stderr, "Invalid or missing nestedComponents array.\n");
                }

                output += "</div>";
            } else {
                fmt::print(stderr, "Unsupported component type: {}\n", type);
                output = "<!-- Unsupported component type: " + type + " -->";
            }

            return output;
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
            View view;
            view.setName(it.key());

            // Parse components
            std::vector<Component> components;
            for (const auto &componentJson : it.value()["components"]) {
                Component component;

                // Asegurarse de que 'type' sea una cadena y convertirlo a ComponentType
                if (componentJson.contains("type") && componentJson["type"].is_string()) {
                    std::string typeStr = componentJson["type"].get<std::string>();
                    component.setType(stringToComponentType(typeStr));
                } else {
                    fmt::print(stderr, "Error: 'type' in 'components' must be a string.\n");
                    continue;
                }

                // Parse props
                std::map<std::string, std::string> props;
                for (auto propIt = componentJson["props"].begin();
                     propIt != componentJson["props"].end();
                     ++propIt) {
                    if (propIt.value().is_string()) {
                        props[propIt.key()] = propIt.value().get<std::string>();
                    } else {
                        fmt::print(stderr,
                                   "Error: Property value for '{}' must be a string.\n",
                                   propIt.key());
                    }
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

            // Convertir ComponentType a cadena antes de asignarlo al JSON
            std::string typeStr = componentTypeToString(component.getType());
            componentJson["type"] = typeStr;

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
    // Cargar el archivo frontend.json existente
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

    // Verificar si el componente ya existe en el array de vistas
    bool componentExists = false;
    for (auto &view : frontendJson["views"]) {
        if (view.contains(componentName)) {
            componentExists = true;
            break;
        }
    }

    // Si el componente no existe, agregarlo al JSON
    if (!componentExists) {
        nlohmann::json newView;
        nlohmann::json newComponent;

        // Usar el tipo de componente "Header H1" por defecto
        ComponentType defaultType = ComponentType::HeaderH1;
        newComponent["type"] = componentTypeToString(defaultType);

        // Obtener las propiedades por defecto del tipo de componente
        auto it = componentPropertiesMap.find(defaultType);
        if (it != componentPropertiesMap.end()) {
            for (const auto &prop : it->second) {
                newComponent["props"][prop.first] = prop.second;
            }
        }

        // Agregar el nuevo componente a la nueva vista
        newView[componentName]["components"].push_back(newComponent);
        frontendJson["views"].push_back(newView);

        // Verificar si la ruta ya existe antes de agregarla
        bool routeExists = false;
        for (const auto &route : frontendJson["routes"]) {
            if (route["component"] == componentName) {
                routeExists = true;
                break;
            }
        }

        // Si la ruta no existe, agregar una nueva ruta
        if (!routeExists) {
            nlohmann::json newRoute;
            newRoute["path"] = "/" + componentName;
            newRoute["component"] = componentName;
            frontendJson["routes"].push_back(newRoute);
        }
    }

    // Guardar el archivo frontend.json actualizado
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
    nlohmann::json data;

    // Inserta el nombre del componente en el contexto de Inja
    data["component"] = componentName;

    // Agrega los componentes actuales si existen
    auto it = std::find_if(views.begin(), views.end(), [&componentName](const View &view) {
        return view.getName() == componentName;
    });

    data["components"] = nlohmann::json::array(); // Asegúrate de inicializar el array

    // Si la vista ya existe, agrega sus componentes al contexto
    if (it != views.end()) {
        for (const auto &component : it->getComponents()) {
            nlohmann::json componentJson;
            componentJson["type"] = component.getType();

            // Agrega las props si existen
            nlohmann::json propsJson;
            for (const auto &prop : component.getProps()) {
                propsJson[prop.first] = prop.second;
            }
            componentJson["props"] = propsJson;

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

    std::string outputPath = projectPath + "/frontend/src/views/" + componentName + ".tsx";

    try {
        // Renderizar con Inja usando el contenido del archivo como una cadena
        std::string result = env.render(templateString, data);
        FileUtils::writeFile(outputPath, result);

        updateFrontendJson(componentName); // Actualizar el frontend.json
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error generating component base for {}: {}\n", componentName, e.what());
        return false;
    }

    fmt::print("Component base generated successfully for {}\n", componentName);
    return true;
}

bool FrontendGenerator::generateMain()
{
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
            if (!generateComponentBase(route.getComponent())) {
                fmt::print(stderr,
                           "Failed to generate component base for {}\n",
                           route.getComponent());
                return false;
            }

            // Agregar la nueva vista al vector de vistas
            View newView(route.getComponent());
            views.push_back(newView);
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

bool FrontendGenerator::generateViews()
{
    for (const auto &view : views) {
        nlohmann::json data;
        data["component"] = view.getName();
        data["components"] = nlohmann::json::array();

        for (const auto &component : view.getComponents()) {
            nlohmann::json componentJson;
            componentJson["type"] = componentTypeToString(component.getType());

            // Agregar las props del componente
            nlohmann::json propsJson;
            for (const auto &prop : component.getProps()) {
                propsJson[prop.first] = prop.second;
            }
            componentJson["props"] = propsJson;

            data["components"].push_back(componentJson);
        }

        // Cargar la plantilla de vista
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
        std::string outputPath = projectPath + "/frontend/src/views/" + view.getName() + ".tsx";

        try {
            // Renderizar con Inja usando el contexto actualizado
            std::string result = env.render(templateString, data);
            FileUtils::writeFile(outputPath, result);
        } catch (const std::exception &e) {
            fmt::print(stderr, "Error generating view for {}: {}\n", view.getName(), e.what());
            return false;
        }
    }

    return true;
}

bool FrontendGenerator::generateFrontendCode()
{
    if (!generateMain())
        return false;

    if (!generateViews())
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
