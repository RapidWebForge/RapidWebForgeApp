#include "codeworker.h"
#include <QDebug>

CodeWorker::CodeWorker() {}

CodeWorker::CodeWorker(CodeGenerator *codeGenerator, QObject *parent)
    : QObject(parent)
    , codeGenerator(codeGenerator)
    , success(true)
{}

void CodeWorker::process()
{
    if (!codeGenerator->backendGenerator.updateBackendCode()) {
        qDebug() << "fail in backend";
        success = false;
    }

    if (!codeGenerator->frontendGenerator.updateFrontendCode()) {
        qDebug() << "fail in frontend";
        success = false;
    }

    emit finished(success);
}
