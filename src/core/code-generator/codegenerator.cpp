#include "codegenerator.h"
#include <QDebug>
#include <QFile>
#include "../../models/time-chrono/timechrono.h"
#include "../../utils/ziphelper/ziphelper.h"
#include <boost/uuid/uuid_io.hpp>
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

    if (zipData.isEmpty()) {
        fmt::print(stderr, "Zip file is empty or could not be read: {}\n", zipPath);
        return false;
    }

    if (!ZipHelper::unzip(zipData, extractPath)) {
        fmt::print(stderr, "Failed to unzip base project to path: {}\n", extractPath);
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
    frontendJson["views"] = nlohmann::json::array();
    frontendJson["custom"] = nlohmann::json::array();

    // Rutas iniciales

    // Vista inicial
    nlohmann::json homeViewJson;
    nlohmann::json homeH1Json;
    nlohmann::json homeH1PropsJson;
    homeViewJson["components"] = nlohmann::json::array();

    Section homeView("Home", "/");
    Component newHeaderH1(ComponentType::HeaderH1);
    // homeView.addChild(newHeaderH1);

    homeH1Json["type"] = componentTypeToString(newHeaderH1.getType());
    homeH1Json["id"] = boost::uuids::to_string(newHeaderH1.getId());
    homeH1Json["createdOn"] = timePointToString(newHeaderH1.getCreatedOn());
    homeH1Json["updatedOn"] = timePointToString(newHeaderH1.getUpdatedOn());
    homeH1PropsJson["text"] = "Home View";
    homeH1PropsJson["class"] = "";
    homeH1Json["props"] = homeH1PropsJson;

    homeViewJson["components"].push_back(homeH1Json);
    homeViewJson["path"] = homeView.getPath();
    homeViewJson["name"] = homeView.getName();
    homeViewJson["id"] = boost::uuids::to_string(homeView.getId());
    homeViewJson["createdOn"] = timePointToString(homeView.getCreatedOn());
    homeViewJson["updatedOn"] = timePointToString(homeView.getUpdatedOn());

    frontendJson["views"].push_back(homeViewJson);

    if (!createJsonFile(this->project.getPath() + "/frontend.json", frontendJson)) {
        return false;
    }

    fmt::print("Base Frontend project created successfully\n");
    return true;
}

// Get
std::string CodeGenerator::getProjectPath() const
{
    return project.getPath();
}
