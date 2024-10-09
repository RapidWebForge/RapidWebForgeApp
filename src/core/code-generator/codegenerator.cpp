#include "codegenerator.h"
#include <QFile>
#include "../../utils/ziphelper/ziphelper.h"
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <nlohmann/json.hpp>

CodeGenerator::CodeGenerator(const Project &project)
    : project(project)
    , backendGenerator(project.getPath(), project.getDatabaseData())
{}

bool CodeGenerator::createBaseBackendProject()
{
    std::string zipPath = ":/project_templates/backend";
    QFile zipFile(QString::fromStdString(zipPath));
    if (!zipFile.open(QIODevice::ReadOnly)) {
        fmt::print(stderr, "Unable to open zip file: {}\n", zipFile.errorString().toStdString());
        return false;
    }

    // Read zip content
    QByteArray zipData = zipFile.readAll();
    zipFile.close();
    std::string extractPath = this->project.getPath() + "/backend";

    // Create backend folder
    if (!std::filesystem::exists(extractPath)) {
        try {
            std::filesystem::create_directories(extractPath);
            fmt::print("Created directory: {}\n", extractPath);
        } catch (const std::filesystem::filesystem_error &e) {
            fmt::print(stderr, "Failed to create directory: {}\n", e.what());
            return false;
        }
    }

    // Unzip file
    if (!ZipHelper::unzip(zipData, extractPath)) {
        fmt::print(stderr, "Failed to unzip base project\n");
        return false;
    }

    // Create backend.json
    nlohmann::json backendJson;
    backendJson["transactions"] = nlohmann::json::array();

    std::ofstream jsonFile(this->project.getPath() + "/backend.json");
    if (!jsonFile.is_open()) {
        fmt::print(stderr, "Failed to create backend.json\n");
        return false;
    }

    jsonFile << backendJson.dump(2);
    jsonFile.close();

    fmt::print("Base project created successfully\n");

    return true;
}

bool CodeGenerator::createBaseFrontendProject()
{
    return true;
}
