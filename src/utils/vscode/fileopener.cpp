#include "FileOpener.h"
#include <boost/process.hpp>  // 📌 Usar Boost.Process
#include <QProcess>
#include <QDebug>
#include <QFileInfo>  // 📌 Agrega esta línea
#include <QOperatingSystemVersion>  // 📌 Detectar sistema operativo
#include <iostream>

namespace bp = boost::process;



bool FileOpener::openInVSCode(const std::string &folderPath, const std::string &filePath) {
    try {
        std::string command;

// 📌 Detectar sistema operativo y formar comando adecuado
#ifdef _WIN32
        command = "code \"" + folderPath + "\"";  // ✅ Comando para Windows
        if (!filePath.empty()) {
            command += " -g \"" + filePath + "\"";  // ✅ Abrir archivo específico
        }
#else
        command = "/usr/local/bin/code \"" + folderPath + "\""; // ✅ Comando para Mac/Linux
        if (!filePath.empty()) {
            command += " -g \"" + filePath + "\"";  // ✅ Abrir archivo específico
        }
#endif

        qDebug() << "🖥 Ejecutando comando: " << QString::fromStdString(command);
        bp::child c(command, bp::std_out > bp::null, bp::std_err > bp::null);
        c.wait();  // ✅ Espera a que VS Code se abra correctamente

        return c.exit_code() == 0;
    } catch (const std::exception &e) {
        std::cerr << "❌ Error al ejecutar VS Code: " << e.what() << std::endl;
        return false;
    }
}
