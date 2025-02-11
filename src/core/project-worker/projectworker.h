#ifndef PROJECTWORKER_H
#define PROJECTWORKER_H

#include <QObject>
#include "../../models/project/project.h"

class ProjectWorker : public QObject
{
    Q_OBJECT

public:
    ProjectWorker();
    explicit ProjectWorker(Project newProject, QObject *parent = nullptr);

signals:
    void finished();

public slots:
    void process();

private:
    Project newProject;
};

#endif // PROJECTWORKER_H
