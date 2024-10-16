#include "codegenerator.h"
#include <QFile>
#include "../../utils/ziphelper/ziphelper.h"
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <nlohmann/json.hpp>

// Constructor
CodeGenerator::CodeGenerator(const Project &project)
    : project(project)
    , backendGenerator(project.getPath(), project.getDatabaseData())
    , frontendGenerator(project.getPath())
{}

// Función genérica para crear un directorio
bool CodeGenerator::createDirectory(const std::string &path)
{
    if (!std::filesystem::exists(path)) {
        try {
            std::filesystem::create_directories(path);
            fmt::print("Created directory: {}\n", path);
            return true;
        } catch (const std::filesystem::filesystem_error &e) {
            fmt::print(stderr, "Failed to create directory: {}\n", e.what());
            return false;
        }
    }
    return true; // El directorio ya existe
}

// Función genérica para descomprimir un archivo
bool CodeGenerator::unzipFile(const std::string &zipPath, const std::string &extractPath)
{
    QFile zipFile(QString::fromStdString(zipPath));
    if (!zipFile.open(QIODevice::ReadOnly)) {
        fmt::print(stderr, "Unable to open zip file: {}\n", zipFile.errorString().toStdString());
        return false;
    }

    // Leer el contenido del archivo ZIP
    QByteArray zipData = zipFile.readAll();
    zipFile.close();

    // Descomprimir archivo
    if (!ZipHelper::unzip(zipData, extractPath)) {
        fmt::print(stderr, "Failed to unzip base project\n");
        return false;
    }

    return true;
}

// Función genérica para crear un archivo JSON
bool CodeGenerator::createJsonFile(const std::string &filePath, const nlohmann::json &jsonData)
{
    std::ofstream jsonFile(filePath);
    if (!jsonFile.is_open()) {
        fmt::print(stderr, "Failed to create file: {}\n", filePath);
        return false;
    }

    jsonFile << jsonData.dump(2); // Dump JSON con indentación
    jsonFile.close();
    return true;
}

// Creación del proyecto base de backend
bool CodeGenerator::createBaseBackendProject()
{
    std::string extractPath = this->project.getPath() + "/backend";
    if (!createDirectory(extractPath)) {
        return false;
    }

    // Unzip backend template
    if (!unzipFile(":/project_templates/backend", extractPath)) {
        return false;
    }

    // Crear el backend.json
    nlohmann::json backendJson;
    backendJson["transactions"] = nlohmann::json::array();

    if (!createJsonFile(this->project.getPath() + "/backend.json", backendJson)) {
        return false;
    }

    fmt::print("Base Backend project created successfully\n");
    return true;
}

// Creación del proyecto base de frontend
bool CodeGenerator::createBaseFrontendProject()
{
    std::string extractPath = this->project.getPath() + "/frontend";
    if (!createDirectory(extractPath)) {
        return false;
    }

    // Unzip frontend template
    if (!unzipFile(":/project_templates/frontend", extractPath)) {
        return false;
    }

    // Crear el frontend.json
    nlohmann::json frontendJson;
    frontendJson["routes"] = nlohmann::json::array();
    frontendJson["views"] = nlohmann::json::array();

    // Rutas iniciales
    nlohmann::json homeRouteJson;
    homeRouteJson["path"] = "/";
    homeRouteJson["component"] = "Home";
    frontendJson["routes"].push_back(homeRouteJson);

    // Vista inicial
    nlohmann::json homeViewJson;
    nlohmann::json homeComponentsJson;
    nlohmann::json homeH1Json;
    nlohmann::json homePropsJson;
    homeComponentsJson["components"] = nlohmann::json::array();
    homeH1Json["type"] = "H1";
    homePropsJson["text"] = "Home View";
    homeH1Json["props"] = homePropsJson;
    homeComponentsJson["components"].push_back(homeH1Json);
    homeViewJson["Home"] = homeComponentsJson;
    frontendJson["views"].push_back(homeViewJson);

    if (!createJsonFile(this->project.getPath() + "/frontend.json", frontendJson)) {
        return false;
    }

    fmt::print("Base Frontend project created successfully\n");
    return true;
}
