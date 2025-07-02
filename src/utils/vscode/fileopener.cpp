#include "fileopener.h"
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QOperatingSystemVersion>
#include <QProcess>

bool FileOpener::openInVSCode(const std::string &folderPath, const std::string &filePath)
{
    // Determinar el ejecutable de VSCode según el SO
    QString program;
#ifdef Q_OS_WIN
    program = "code"; // Asume que 'code' está en PATH
#elif defined(Q_OS_MAC)
    program = "/usr/local/bin/code"; // Ruta típica en macOS
#else
    program = "code"; // Linux y otros UNIX
#endif

    // Construir argumentos
    QStringList arguments;
    if (!folderPath.empty()) {
        arguments << QString::fromStdString(folderPath);
    }
    if (!filePath.empty()) {
        arguments << "-g" << QString::fromStdString(filePath);
    }

    qDebug() << "🖥 Ejecutando VSCode:" << program << arguments;

    // Usar startDetached para no bloquear la aplicación
    bool launched = QProcess::startDetached(program, arguments);
    if (!launched) {
        qWarning() << "❌ No se pudo iniciar VSCode en" << program;
        QMessageBox::critical(nullptr,
                              "ERROR",
                              "No tienes instalado VSCode para visualizar el código");
    }
    return launched;
}
