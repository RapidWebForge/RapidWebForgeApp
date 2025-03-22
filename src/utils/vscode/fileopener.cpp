#include "FileOpener.h"
#include <QDebug>
#include <QFileInfo>               // 📌 Agrega esta línea
#include <QOperatingSystemVersion> // 📌 Detectar sistema operativo
#include <QProcess>
#include <boost/process.hpp> // 📌 Usar Boost.Process
#include <iostream>

namespace bp = boost::process;

bool FileOpener::openInVSCode(const std::string &folderPath, const std::string &filePath)
{
    try {
        std::string command;

// 📌 Detectar sistema operativo y formar comando adecuado
#ifdef _WIN32
        command = "code \"" + folderPath + "\""; // ✅ Comando para Windows
        if (!filePath.empty()) {
            command += " -g \"" + filePath + "\""; // ✅ Abrir archivo específico
        }
#else
        command = "/usr/local/bin/code \"" + folderPath + "\""; // ✅ Comando para Mac/Linux
        if (!filePath.empty()) {
            command += " -g \"" + filePath + "\""; // ✅ Abrir archivo específico
        }
#endif
        qDebug() << "🖥 Ejecutando comando:" << QString::fromStdString(command);

        // Ejecutar el comando como si fuera terminal completa
        bp::child c(command, bp::shell, bp::std_out > bp::null, bp::std_err > bp::null);
        c.wait(); // Esperar (opcional)
        return c.exit_code() == 0;
    } catch (const std::exception &e) {
        std::cerr << "❌ Error al ejecutar VS Code: " << e.what() << std::endl;
        return false;
    }
}
