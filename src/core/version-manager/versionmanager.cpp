#include "versionmanager.h"

#include <fstream>
#include <iostream>

VersionManager::VersionManager(const std::string &projectPath)
    : projectPath(projectPath)
{}

void VersionManager::initializeRepository()
{
    // Inicializar el repositorio Git
    std::string command = "cd " + projectPath + " && git init";
    int result = system(command.c_str());

    if (result == 0) {
        std::cout << "Git repository initialized successfully in: " << projectPath << std::endl;

        // Crear el archivo .gitignore en el directorio del proyecto
        std::string gitignorePath = projectPath + "/.gitignore";
        std::ofstream gitignoreFile(gitignorePath);

        if (gitignoreFile.is_open()) {
            // Agregar las reglas comunes al .gitignore
            gitignoreFile << "# RapidWebForge folders\n";
            gitignoreFile << "\backend\n";
            gitignoreFile << "\frontend\n";

            gitignoreFile.close();
            std::cout << ".gitignore file created successfully at: " << gitignorePath << std::endl;
        } else {
            std::cerr << "Failed to create .gitignore file." << std::endl;
        }
    } else {
        std::cerr << "Failed to initialize Git repository." << std::endl;
    }
}

void VersionManager::createVersion(const std::string &version)  {
    // TODO: Implement this method
}

void VersionManager::deleteVersion(const std::string &version) {
    // TODO: Implement this method
}
