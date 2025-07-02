#include "filewatcher.h"
#include <QDebug>
#include <QDir>
#include "../../utils/vscode/fileopener.h"

FileWatcher::FileWatcher(QObject *parent) : QObject(parent) {
    watcher = new QFileSystemWatcher(this);

    connect(watcher, &QFileSystemWatcher::fileChanged, this, [=](const QString &path) {
        qDebug() << "Archivo modificado: " << path;
        std::string filePath = path.toStdString(); // Convertir QString a std::string
        if (filePath.find(".json") == std::string::npos)
            FileOpener::openInVSCode(filePath);
    });
}

void FileWatcher::watchProjectFiles(const QString &projectPath) {
    QDir dir(projectPath);
    if (!dir.exists()) {
        qDebug() << "❌ La carpeta de monitoreo no existe: " << projectPath;
        return;
    }

    QStringList fileList = dir.entryList(QDir::Files);
    if (fileList.isEmpty()) {
        qDebug() << "⚠ No hay archivos en la carpeta: " << projectPath;
    }

    for (const QString &file : fileList) {
        QString filePath = projectPath + "/" + file;
        watcher->addPath(filePath);
        qDebug() << "👀 Monitoreando archivo: " << filePath;
    }
}
QString FileWatcher::getLastModifiedFile() {
    qDebug() << "📂 Último archivo modificado: " << lastModifiedFile;

    return lastModifiedFile;
}
std::string FileWatcher::getLastModifiedFileInFolder(const std::string &folderPath) {
    QDir dir(QString::fromStdString(folderPath));
    if (!dir.exists()) {
        qDebug() << "❌ Carpeta no encontrada: " << QString::fromStdString(folderPath);
        return "";
    }

    // Filtrar solo archivos dentro de la carpeta (excluir backend.json y frontend.json)
    QFileInfoList files = dir.entryInfoList(QDir::Files, QDir::Time | QDir::Reversed);

    for (const QFileInfo &file : files) {
        if (file.fileName() != "backend.json" && file.fileName() != "frontend.json") {
            qDebug() << "Último archivo encontrado en carpeta: " << file.absoluteFilePath();
            return file.absoluteFilePath().toStdString();
        }
    }

    qDebug() << "⚠ No se encontraron archivos en: " << QString::fromStdString(folderPath);
    return "";
}
