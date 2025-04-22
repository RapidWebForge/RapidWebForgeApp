#include "deployworker.h"
#include <QMessageBox>

DeployWorker::DeployWorker(const std::string &projectPath,
                           const std::string &ngInxPath,
                           const std::string &bunPath,
                           QObject *parent)
    : QObject(parent)
    , deployManager(DeployManager(projectPath, ngInxPath, bunPath))
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
