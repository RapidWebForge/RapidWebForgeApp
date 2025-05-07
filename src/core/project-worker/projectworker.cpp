#include "projectworker.h"
#include <QDebug>
#include "../code-generator/codegenerator.h"
#include "../project-manager/projectmanager.h"
#include "../version-manager/versionmanager.h"

ProjectWorker::ProjectWorker() {}

ProjectWorker::ProjectWorker(Project newProject, QObject *parent)
    : QObject(parent)
    , newProject(newProject)
{}

void ProjectWorker::process()
{
    // Create the project
    ProjectManager projectManager;
    projectManager.createProject(newProject);

    CodeGenerator codeGenerator(newProject);
    if (codeGenerator.createRunEditor()) {
        if (codeGenerator.createBaseBackendProject())
            if (codeGenerator.createBaseFrontendProject() && newProject.getBaseProject())
                codeGenerator.createApplication();
    }

    // Inicializar repositorio Git si versions está habilitado
    if (newProject.getVersions()) {
        VersionManager versionManager(newProject.getPath());
        versionManager.initializeRepository();
    }

    emit finished(); // Señal para indicar que terminó
}
