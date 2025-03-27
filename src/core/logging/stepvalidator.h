#ifndef STEPVALIDATOR_H
#define STEPVALIDATOR_H
#include <nlohmann/json.hpp> // Biblioteca JSON
#include <string>

class StepValidator
{
public:
    StepValidator(const std::string &stepsFilePath, const std::string &logFilePath);
    bool isStepCompleted(const std::string &action, const std::string &component);
    int getCurrentStep();                     // Devuelve el paso actual
    std::string getStepDescription(int step); // Descripción del paso

private:
    std::string stepsFilePath;
    std::string logFilePath;
    nlohmann::json readStepsFile();
    nlohmann::json readLogFile();
};

#endif // STEPVALIDATOR_H
