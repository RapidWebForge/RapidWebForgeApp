#ifndef DEPLOYWORKER_H
#define DEPLOYWORKER_H

#include <QObject>
#include <QString>
#include "../../core/deploy-manager/deploymanager.h"

class DeployWorker : public QObject
{
    Q_OBJECT

public:
    explicit DeployWorker(const std::string &projectPath,
                          const std::string &ngInxPath,
                          const std::string &bunPath,
                          QObject *parent = nullptr);

signals:
    void finished(QString errorMessage = {});

public slots:
    void process();

private:
    DeployManager deployManager;
};

#endif // DEPLOYWORKER_H
