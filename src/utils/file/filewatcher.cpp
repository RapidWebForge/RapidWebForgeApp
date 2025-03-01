#include "FileWatcher.h"
#include "../../utils/vscode/fileopener.h"
#include <QDir>
#include <QDebug>

FileWatcher::FileWatcher(QObject *parent) : QObject(parent) {
    watcher = new QFileSystemWatcher(this);

    connect(watcher, &QFileSystemWatcher::fileChanged, this, [=](const QString &path) {
        qDebug() << "Archivo modificado: " << path;
        std::string filePath = path.toStdString();  // ✅ Convertir QString a std::string
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
