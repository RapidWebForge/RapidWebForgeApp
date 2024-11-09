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

bool isNginxRunning()
{
    bp::ipstream pipe_stream;
    bp::child c("tasklist /fi \"imagename eq nginx.exe\"", bp::std_out > pipe_stream);
    c.wait();

    std::string line;
    while (std::getline(pipe_stream, line)) {
        if (line.find("nginx.exe") != std::string::npos) {
            // Si encontramos "nginx.exe" en la salida, significa que el proceso está en ejecución
            return true;
        }
    }
    return false;
}

void DeployManager::start(const std::string bunPath)
{
    createNginxConfig(9000, 3000);

    if (!isNginxRunning()) {
        qDebug() << "Nginx no está en ejecución, iniciando Nginx...";
        try {
            bp::child nginx = runNgInx(); // Asegúrate de tener esta función implementada
            nginx.detach();
        } catch (const std::exception &e) {
            qDebug() << "Error al iniciar Nginx:" << e.what();
        }
    } else {
        qDebug() << "Nginx ya está en ejecución.";
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
