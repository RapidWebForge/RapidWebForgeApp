#include "actionloggerjson.h"
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

ActionLoggerJson::ActionLoggerJson(const std::string &logFilePath)
    : logFilePath(logFilePath)
{
    ensureLogFileExists();
}

void ActionLoggerJson::logAction(const std::string &action, const std::string &componentID)
{
    // Leer el archivo JSON actual
    json logData = readLogFile();

    // Obtener fecha y hora actual
    std::time_t now = std::time(nullptr);
    char timeStr[20];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%S", std::localtime(&now));

    // Crear un nuevo registro de log
    json logEntry = {{"action", action}, {"component", componentID}, {"timestamp", timeStr}};

    // Agregar el nuevo registro al JSON
    logData.push_back(logEntry);

    // Escribir el JSON actualizado de vuelta al archivo
    writeLogFile(logData);
}

json ActionLoggerJson::readLogFile()
{
    // Leer el archivo JSON existente
    std::ifstream logFile(logFilePath);
    if (logFile.is_open()) {
        json logData;
        logFile >> logData;
        logFile.close();
        return logData;
    } else {
        // Si el archivo no se puede leer, devolver un JSON vacío
        return json::array();
    }
}

void ActionLoggerJson::writeLogFile(const json &logData)
{
    // Escribir el JSON al archivo
    std::ofstream logFile(logFilePath);
    if (logFile.is_open()) {
        logFile << logData.dump(4); // Escribir con una indentación de 4 espacios
        logFile.close();
    } else {
        std::cerr << "Error: No se pudo escribir en el archivo de log JSON." << std::endl;
    }
}

void ActionLoggerJson::ensureLogFileExists()
{
    // Crear directorio si no existe
    std::filesystem::path logPath = std::filesystem::path(logFilePath).parent_path();
    if (!std::filesystem::exists(logPath)) {
        std::filesystem::create_directories(logPath);
    }

    // Crear archivo JSON si no existe
    if (!std::filesystem::exists(logFilePath)) {
        std::ofstream logFile(logFilePath);
        if (logFile.is_open()) {
            logFile << "[]"; // Inicializar el archivo como un array vacío
            logFile.close();
        } else {
            std::cerr << "Error: No se pudo crear el archivo de log JSON." << std::endl;
        }
    }
}
