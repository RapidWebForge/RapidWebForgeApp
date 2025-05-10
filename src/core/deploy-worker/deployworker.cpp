#include "deployworker.h"
#include <QMessageBox>

DeployWorker::DeployWorker(const std::string &projectPath, QObject *parent)
    : QObject(parent)
    , deployManager(DeployManager(projectPath))
{}

void DeployWorker::process()
{
    try {
        // Iniciar el despliegue
        deployManager.start();
        emit finished();
    } catch (const std::exception &e) {
        emit finished(QString::fromStdString(e.what()));
    }
}

DeployManager &DeployWorker::getDeployManager()
{
    return deployManager;
}
