#ifndef ACTIONLOGGERJSON_H
#define ACTIONLOGGERJSON_H

#include <nlohmann/json.hpp> // Incluye la biblioteca JSON
#include <string>

class ActionLoggerJson
{
public:
    explicit ActionLoggerJson(const std::string &logFilePath);
    void logAction(const std::string &action, const std::string &componentID);

private:
    std::string logFilePath;
    nlohmann::json readLogFile();                     // Leer archivo JSON existente
    void writeLogFile(const nlohmann::json &logData); // Escribir archivo JSON
    void ensureLogFileExists();
    void resetLogFile();
};
#endif // ACTIONLOGGERJSON_H
