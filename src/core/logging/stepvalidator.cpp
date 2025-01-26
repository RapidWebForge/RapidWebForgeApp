#include "stepvalidator.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

StepValidator::StepValidator(const std::string &stepsFilePath, const std::string &logFilePath)
    : stepsFilePath(stepsFilePath)
    , logFilePath(logFilePath)
{}
bool StepValidator::isStepCompleted(const std::string &action, const std::string &component)
{
    json steps = readStepsFile();
    json logs = readLogFile();

    if (logs.empty() || steps.empty()) {
        return false; // No hay pasos o acciones registrados
    }

    // Buscar el último paso completado en los logs
    for (auto it = logs.rbegin(); it != logs.rend(); ++it) {
        if ((*it)["action"] == action && (*it)["component"] == component) {
            return true; // El paso ha sido completado
        }
    }

    return false; // El paso no ha sido completado
}

int StepValidator::getCurrentStep()
{
    json steps = readStepsFile();
    json logs = readLogFile();

    if (logs.empty() || steps.empty()) {
        return 1; // Comienza en el paso 1 si no hay logs
    }

    for (size_t i = 0; i < steps.size(); ++i) {
        if (i >= logs.size() || steps[i]["action"] != logs[i]["action"]
            || steps[i]["component"] != logs[i]["component"]) {
            return steps[i]["step"];
        }
    }

    return steps.size() + 1; // Todos los pasos completados
}

std::string StepValidator::getStepDescription(int step)
{
    json steps = readStepsFile();
    for (const auto &s : steps) {
        if (s["step"] == step) {
            return s["description"];
        }
    }
    return "Step not found.";
}

json StepValidator::readStepsFile()
{
    std::ifstream file(stepsFilePath);
    if (file.is_open()) {
        json steps;
        file >> steps;
        file.close();
        return steps;
    }
    return json::array();
}

json StepValidator::readLogFile()
{
    std::ifstream file(logFilePath);
    if (file.is_open()) {
        json logs;
        file >> logs;
        file.close();
        return logs;
    }
    return json::array();
}
