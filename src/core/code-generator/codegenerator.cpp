#include "codegenerator.h"
#include <QFile>
#include "../../utils/ziphelper/ziphelper.h"
#include <fmt/core.h>
#include <fstream>
#include <nlohmann/json.hpp>

CodeGenerator::CodeGenerator(const Project &project)
    : backendGenerator(project.getPath())
{}

bool CodeGenerator::createBaseBackendProject()
{
    // std::string zipPath = ":/project_templates/backend";
    // std::string extractPath = this->project.getPath() + "/backend";
    QFile zipFile(":/project_templates/backend.zip");
    if (!zipFile.open(QIODevice::ReadOnly)) {
        fmt::print(stderr, "Unable to open zip file: {}\n", zipFile.errorString().toStdString());
        return false;
    }

    QByteArray zipData = zipFile.readAll(); // Leer el contenido del archivo ZIP
    zipFile.close();
    std::string extractPath = this->project.getPath() + "/backend";

    if (!ZipHelper::unzip(zipData, extractPath)) {
        fmt::print(stderr, "Failed to unzip base project\n");
        return false;
    }

    // Create backend.json
    nlohmann::json backendJson;
    backendJson["transactions"] = nlohmann::json::array();

    std::ofstream jsonFile(this->project.getPath() + "/backend/backend.json");
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
