#include "deploymanager.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include "../../utils/file/fileutiils.h"
#include <filesystem>
#include <fmt/core.h>
#include <inja/inja.hpp>
#include <iostream>
#include <string>
#include <thread>

DeployManager::DeployManager(const std::string projectPath, const std::string ngInxPath)
    : projectPath(projectPath)
    , ngInxPath(ngInxPath)
    , ngInxDirectory(std::filesystem::path(ngInxPath).parent_path().string())
    , configFilePath(ngInxDirectory + "\\conf\\nginx.conf")
{}

DeployManager::~DeployManager()
{
    // kill();
}

bp::child DeployManager::runBackend(const std::string bunPath)
{
    std::string command = bunPath + " run ./server.js";
    return bp::child(command, bp::start_dir = this->projectPath + "\\backend");
}

bp::child DeployManager::runFrontend(const std::string bunPath)
{
    std::string command = bunPath + " run dev";
    return bp::child(command, bp::start_dir = this->projectPath + "\\frontend");
}

bp::child DeployManager::runNgInx()
{
    return bp::child(ngInxPath, bp::start_dir = ngInxDirectory);
}

void DeployManager::start(const std::string bunPath)
{
    std::string pidFilePath = ngInxDirectory + "\\logs\\nginx.pid";

    createNginxConfig(9000, 3000);

    try {
        if (!std::filesystem::exists(pidFilePath)) {
            qDebug() << "El archivo PID no existe, iniciando Nginx...";
            bp::child nginx = runNgInx();
            nginx.detach();
        }
    } catch (const std::filesystem::filesystem_error &e) {
        qDebug() << "Error al verificar la existencia del archivo PID:" << e.what();
    }

    // Lanza el backend en un hilo
    bp::child backend(runBackend(bunPath));
    std::thread([backend = std::move(backend)]() mutable { backend.detach(); }).detach();

    // Lanza el frontend en un hilo
    bp::child frontend(runFrontend(bunPath));
    std::thread([frontend = std::move(frontend)]() mutable { frontend.detach(); }).detach();
}

void DeployManager::kill()
{
    bp::system(ngInxPath + " -c " + configFilePath + " -p " + ngInxDirectory + " -s stop");
}

void DeployManager::reload()
{
    bp::system(ngInxPath + " -c " + configFilePath + " -p " + ngInxDirectory + " -s reload");
}

void DeployManager::createNginxConfig(int frontendPort, int backendPort)
{
    inja::Environment env;
    std::string templatePath = ":/inja/nginx/nginx_conf";

    QFile file(QString::fromStdString(templatePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmt::print(stderr, "Unable to open template file from resource: {}\n", templatePath);
        return;
    }

    QTextStream in(&file);
    QString templateContent = in.readAll();
    file.close();

    std::string templateString = templateContent.toStdString();

    try {
        // Definir los valores de las variables en un json
        nlohmann::json data;
        data["frontend_port"] = frontendPort;
        data["backend_port"] = backendPort;

        // Renderizar la plantilla con los datos
        std::string result = env.render(templateString, data);

        FileUtils::writeFile(configFilePath, result);
    } catch (const std::exception &e) {
        std::cerr << "Error al procesar la plantilla: " << e.what() << std::endl;
    }
}
