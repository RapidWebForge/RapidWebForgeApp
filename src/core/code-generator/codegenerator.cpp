#include "codegenerator.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include "../../models/time-chrono/timechrono.h"
#include "../../utils/file/fileutiils.h"
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

bool CodeGenerator::createRunEditor()
{
    // Create runEditor.js
    QString resourcePath = ":/project_templates/runEditor";
    QString destinationPath = QDir(QString::fromStdString(this->project.getPath()))
                                  .filePath("runEditor.js");

    QFile file(resourcePath);
    if (file.open(QIODevice::ReadOnly)) {
        QFile outFile(destinationPath);
        if (outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            outFile.write(file.readAll());
            outFile.close();
        } else {
            qWarning() << "❌ Could not open output file:" << destinationPath;
            return false;
        }
        file.close();
    } else {
        qWarning() << "❌ Could not open resource file:" << resourcePath;
        return false;
    }

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

    // Crear el .env
    std::string templatePath = ":/inja/backend/env";
    std::string outputPath = this->project.getPath() + "/backend/.env";

    inja::Environment env;
    nlohmann::json data;

    data["database"] = this->project.getDatabaseData().getDatabaseName();
    data["user"] = this->project.getDatabaseData().getUser();
    data["password"] = this->project.getDatabaseData().getPassword();
    data["port"] = this->project.getBackendPort();

    QFile file(QString::fromStdString(templatePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmt::print(stderr, "Unable to open template file from resource: {}\n", templatePath);
        return false;
    }

    QTextStream in(&file);
    QString templateContent = in.readAll();
    file.close();

    // Convertir el contenido a std::string para usarlo con Inja
    std::string templateString = templateContent.toStdString();

    try {
        // Renderizar con Inja usando el contenido del archivo como una cadena
        std::string result = env.render(templateString, data);
        // Open file in write mode
        std::ofstream file(outputPath);

        // Check open file
        if (!file.is_open()) {
            fmt::print(stderr, "Unable to open file for writing: {}\n", outputPath);
            return false;
        }

        // Write content
        file << result;

        file.close();

        // Optional verification
        if (file.fail()) {
            fmt::print(stderr, "Error while writing and closing the file: {}\n", outputPath);
        }
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error generating file {}: {}\n", ".env", e.what());
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
    // Components vacio
    homeViewJson["components"] = nlohmann::json::array();

    Section homeView("Home", "/");

    homeViewJson["path"] = homeView.getPath();
    homeViewJson["name"] = homeView.getName();
    homeViewJson["id"] = boost::uuids::to_string(homeView.getId());
    homeViewJson["createdOn"] = timePointToString(homeView.getCreatedOn());
    homeViewJson["updatedOn"] = timePointToString(homeView.getUpdatedOn());

    Component mainDiv(ComponentType::Layout);

    nlohmann::json mainDivJson;
    mainDivJson["id"] = boost::uuids::to_string(mainDiv.getId());
    mainDivJson["createdOn"] = timePointToString(mainDiv.getCreatedOn());
    mainDivJson["updatedOn"] = timePointToString(mainDiv.getUpdatedOn());
    mainDivJson["type"] = componentTypeToString(mainDiv.getType());
    mainDivJson["props"] = nlohmann::json::object();
    mainDivJson["props"]["class"] = "";
    mainDivJson["nestedComponents"] = nlohmann::json::array();

    homeViewJson["components"].push_back(mainDivJson);

    frontendJson["views"].push_back(homeViewJson);

    if (!createJsonFile(this->project.getPath() + "/frontend.json", frontendJson)) {
        return false;
    }

    // Creating Home view

    nlohmann::json data;

    data["id"] = boost::uuids::to_string(mainDiv.getId());
    std::string outputPath = this->project.getPath() + "/frontend/src/views/Home.tsx";

    try {
        // Renderizar con Inja usando el contenido del archivo como una cadena
        inja::Environment env;
        std::string result = env.render(R"(
import React from "react";

export default function Home() {
  return <div data-id="{{id}}"></div>;
}
                                            )",
                                        data);
        FileUtils::writeFile(outputPath, result);
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error generating Home view: {}\n", e.what());
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
