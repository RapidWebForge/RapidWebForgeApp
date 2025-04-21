#include "deploymanager.h"
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QHostAddress>
#include <QProcess>
#include <QTcpSocket>
#include <QTextStream>
#include <QThread>
#include "../../utils/file/fileutiils.h"
#include <filesystem>
#include <fmt/core.h>
#include <inja/inja.hpp>
#include <iostream>
#include <string>

DeployManager::DeployManager(const std::string projectPath, const std::string ngInxPath)
    : projectPath(projectPath)
    , ngInxPath(ngInxPath)
    , ngInxDirectory(QFileInfo(QString::fromStdString(ngInxPath)).absolutePath().toStdString())
    , configFilePath(QDir(QString::fromStdString(projectPath)).filePath("nginx.conf").toStdString())
{}

DeployManager::~DeployManager()
{
    // kill();
}

bool isNginxRunning()
{
    QProcess process;
#ifdef _WIN32
    process.start("tasklist", {"/fi", "imagename eq nginx.exe"});
#elif defined(__APPLE__) || defined(__linux__)
    process.start("pgrep", {"nginx"});
#else
    return false;
#endif
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    return !output.trimmed().isEmpty();
}

bool waitForPort(const QHostAddress &host, quint16 port, int timeoutMs = 10000)
{
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < timeoutMs) {
        QTcpSocket socket;
        socket.connectToHost(host, port);
        if (socket.waitForConnected(100)) {
            socket.disconnectFromHost();
            return true;
        }
        QThread::msleep(200);
    }
    return false;
}

void startInTerminal(const QString &dir, const QString &cmd)
{
#ifdef Q_OS_WIN
    QString program = "cmd.exe";
    QStringList args{"/k", QString("cd /d \"%1\" && %2").arg(dir, cmd)};
    QProcess::startDetached(program, args);
#elif defined(Q_OS_MAC)
    QString script = QString("tell application \"Terminal\"\n"
                             "  do script \"cd '%1' && %2\"\n"
                             "  activate\n"
                             "end tell")
                         .arg(dir, cmd);
    QProcess::startDetached("/usr/bin/osascript", {"-e", script});
#elif defined(Q_OS_LINUX)
    QString program = "gnome-terminal"; // o x-terminal-emulator
    QStringList args{"--", "-e", QString("bash -ic \"cd '%1' && %2; exec bash\"").arg(dir, cmd)};
    QProcess::startDetached(program, args);
#endif
}

void DeployManager::start(const std::string bunPath)
{
    createNginxConfig(9000, 3000);

    QString backendDir = QDir(QString::fromStdString(projectPath)).filePath("backend");
    startInTerminal(backendDir, QString::fromStdString(bunPath) + " run dev");
    QString frontendDir = QDir(QString::fromStdString(projectPath)).filePath("frontend");
    startInTerminal(frontendDir, QString::fromStdString(bunPath) + " run dev");

    // Esperar a que estén listos
    qDebug() << "Esperando backend (3000)...";
    waitForPort(QHostAddress::LocalHost, 3000);
    qDebug() << "Backend disponible.";

    qDebug() << "Esperando frontend (9000)...";
    waitForPort(QHostAddress::LocalHost, 9000);
    qDebug() << "Frontend disponible.";

    // Ahora lanzar nginx

    // NgInx
    if (!isNginxRunning()) {
        qDebug() << "Nginx is not running, starting Nginx...";
        QProcess *nginxProcess = new QProcess();
        nginxProcess->start(QString::fromStdString(ngInxPath),
                            {"-c", QString::fromStdString(configFilePath)});
        QObject::connect(nginxProcess, &QProcess::errorOccurred, [](QProcess::ProcessError error) {
            qDebug() << "Error to start Nginx:" << error;
        });
    } else {
        qDebug() << "Nginx is already running.";
    }
}

void DeployManager::kill()
{
    QStringList args = {"-c", QString::fromStdString(configFilePath), "-s", "stop"};
    QProcess::execute(QString::fromStdString(ngInxPath), args);
}

void DeployManager::reload()
{
    QStringList args = {"-c", QString::fromStdString(configFilePath), "-s", "reload"};
    QProcess::execute(QString::fromStdString(ngInxPath), args);
}

void DeployManager::createNginxConfig(int frontendPort, int backendPort)
{
    QFile configFile(QString::fromStdString(configFilePath));
    if (configFile.exists()) {
        qDebug() << "nginx.conf already exists in:" << QString::fromStdString(configFilePath);
        return; // No volver a crear
    }

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
        nlohmann::json data;
        data["frontend_port"] = frontendPort;
        data["backend_port"] = backendPort;

        std::string result = env.render(templateString, data);

        // Asegurarse de que el directorio exista
        QDir().mkpath(QFileInfo(QString::fromStdString(configFilePath)).absolutePath());

        FileUtils::writeFile(configFilePath, result);
        qDebug() << "nginx.conf create in:" << QString::fromStdString(configFilePath);
    } catch (const std::exception &e) {
        std::cerr << "Error to process the template: " << e.what() << std::endl;
    }
}
