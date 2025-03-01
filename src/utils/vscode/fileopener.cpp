#include "FileOpener.h"
#include <boost/process.hpp>  // 📌 Usar Boost.Process
#include <QProcess>
#include <QDebug>
#include <QFileInfo>  // 📌 Agrega esta línea
#include <QOperatingSystemVersion>  // 📌 Detectar sistema operativo
#include <iostream>

namespace bp = boost::process;


bool FileOpener::openInVSCode(const std::string &path) {
    try {
        std::string command;

#ifdef _WIN32
        command = "code \"" + path + "\"";  // ✅ Comando para Windows
#else
        command = "/usr/local/bin/code \"" + path + "\""; // ✅ Comando para Mac/Linux
#endif

        bp::child c(command, bp::std_out > bp::null, bp::std_err > bp::null);
        c.wait();  // ✅ Espera a que VS Code se abra correctamente

        return c.exit_code() == 0;
    } catch (const std::exception &e) {
        std::cerr << "❌ Error al ejecutar VS Code: " << e.what() << std::endl;
        return false;
    }
}
