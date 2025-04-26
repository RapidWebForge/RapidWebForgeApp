#include "deploymanager.h"
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QHostAddress>
#include <QTcpSocket>
#include <QTextStream>
#include <QThread>
#include "../../utils/file/fileutiils.h"
#include <filesystem>
#include <fmt/core.h>
#include <inja/inja.hpp>
#include <iostream>
#include <string>

DeployManager::DeployManager(const std::string &projectPath,
                             const std::string &ngInxPath,
                             const std::string &bunPath,
                             QObject *parent)
    : QObject(parent)
    , projectPath(projectPath)
    , ngInxPath(ngInxPath)
    , bunPath(bunPath)
    , configFilePath(QDir(QString::fromStdString(projectPath)).filePath("nginx.conf"))
    , ngInxDirectory(QFileInfo(configFilePath).absolutePath())
{}

DeployManager::~DeployManager()
{
    // kill();
}

bool isNginxRunning()
{
    QProcess process;
#ifdef _WIN32
    // process.start("tasklist", {"/fi", "imagename eq nginx.exe"});
    process.start("cmd.exe", {"/c", "tasklist | findstr /i nginx"});
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

bool DeployManager::spawnServer(const QString &dir, quint16 port, QProcess *&handle)
{
    handle = new QProcess(this);
    handle->setWorkingDirectory(dir);
    handle->start(QString::fromStdString(bunPath), {"run", "dev"});
    if (!handle->waitForStarted(10'000))
        return false;
    return waitForPort(QHostAddress::LocalHost, port, 15'000);
}

void DeployManager::spawnTerminal(const QString &dir)
{
    QString script = QString("tell application \"Terminal\"\n"
                             "  do script \"cd '%1' && %2\"\n"
                             "  activate\n"
                             "end tell")
                         .arg(dir, QString::fromStdString(bunPath) + " run dev");
    QProcess::startDetached("/usr/bin/osascript", {"-e", script});
}

void DeployManager::start()
{
#ifdef Q_OS_WIN
    // 1. Matar posibles servidores backend y frontend en puertos 3000 y 9000 (WINDOWS)
    QString portBackend = QString("$conn = Get-NetTCPConnection -LocalPort 3000; "
                                  "if ($conn) { Stop-Process -Id $conn.OwningProcess -Force }");

    QProcess::execute("powershell.exe", {"-NoProfile", "-Command", portBackend});

    QString portFrontend = QString("$conn = Get-NetTCPConnection -LocalPort 9000; "
                                   "if ($conn) { Stop-Process -Id $conn.OwningProcess -Force }");

    QProcess::execute("powershell.exe", {"-NoProfile", "-Command", portFrontend});
#endif

    // 2. Matar nginx (usando el viejo config)
    kill();
    // 3. Crear nuevo archivo nginx.conf
    createNginxConfig(9000, 3000);

    QString backendDir = QDir(QString::fromStdString(projectPath)).filePath("backend");
    QString frontendDir = QDir(QString::fromStdString(projectPath)).filePath("frontend");

#ifdef Q_OS_WIN
    if (!spawnServer(backendDir, 3000, backProcess)) {
        return;
    }
    if (!spawnServer(frontendDir, 9000, frontProcess)) {
        return;
    }
#elif defined(Q_OS_MAC)
    spawnTerminal(backendDir);
    spawnTerminal(frontendDir);
#endif

    // NgInx
    if (!isNginxRunning()) {
        qDebug() << "Nginx is not running, starting Nginx...";

        nginxProcess = new QProcess(this);
        QString fixedConfigPath = QDir::toNativeSeparators(configFilePath);
        QString prefix = QDir::toNativeSeparators(ngInxDirectory);

        nginxProcess->setWorkingDirectory(ngInxDirectory);
        nginxProcess->start(QString::fromStdString(ngInxPath),
                            {"-p", prefix, "-c", fixedConfigPath});

        if (!nginxProcess->waitForStarted(5'000)) {
            qWarning() << "nginx no arrancó en 5 s";
            qWarning() << nginxProcess->readAllStandardError();
        }

        connect(nginxProcess, &QProcess::readyReadStandardError, [=]() {
            qWarning() << nginxProcess->readAllStandardError().trimmed();
        });

    } else {
        qDebug() << "Nginx is already running.";
    }
}

void DeployManager::kill()
{
#ifdef Q_OS_WIN
    // Mata TODOS los procesos nginx.exe
    QProcess::execute("taskkill", {"/F", "/IM", "nginx.exe"});
#else
    QStringList args = {"-s", "stop"};
    QProcess::execute(QString::fromStdString(ngInxPath), args);
#endif
}

void DeployManager::createNginxConfig(int frontendPort, int backendPort)
{
    // Base: directorio donde vamos a colocar nginx.conf, logs/, temp/, etc.
    QString baseDir = QFileInfo(configFilePath).absolutePath();

    // 1) Crear logs/
    QString logsPath = QDir(baseDir).filePath("logs");
    if (QDir().mkpath(logsPath)) {
        qDebug() << "✅ Directorio logs/ creado en:" << logsPath;
    } else {
        qDebug() << "📁 Directorio logs/ ya existe o no se pudo crear en:" << logsPath;
    }

    // 2) Crear todas las carpetas temp que nginx espera
    QStringList tempSubs = {"temp/client_body_temp",
                            "temp/proxy_temp",
                            "temp/fastcgi_temp",
                            "temp/uwsgi_temp",
                            "temp/scgi_temp"};
    for (const QString &sub : tempSubs) {
        QString fullPath = QDir(baseDir).filePath(sub);
        if (QDir().mkpath(fullPath)) {
            qDebug() << "✅ Creada carpeta nginx temp:" << fullPath;
        } else {
            qWarning() << "❌ No se pudo crear carpeta nginx temp:" << fullPath;
        }
    }

    QFile configFile(configFilePath);
    if (configFile.exists()) {
        qDebug() << "nginx.conf already exists in:" << configFilePath;
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
        QDir().mkpath(QFileInfo(configFilePath).absolutePath());

        FileUtils::writeFile(configFilePath.toStdString(), result);
        qDebug() << "nginx.conf create in:" << configFilePath;
    } catch (const std::exception &e) {
        std::cerr << "Error to process the template: " << e.what() << std::endl;
    }
}

// QString fixedDir = QDir::toNativeSeparators(dir);
// QString fullCommand = QString("cd '%1'; %2").arg(fixedDir, cmd);
// QProcess::startDetached("powershell.exe", {"-NoExit", "-Command", fullCommand});
