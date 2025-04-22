#ifndef CODEWORKER_H
#define CODEWORKER_H

#include <QObject>
#include "../../core/code-generator/codegenerator.h"

class CodeWorker : public QObject
{
    Q_OBJECT
public:
    CodeWorker();
    explicit CodeWorker(CodeGenerator *codeGenerator, QObject *parent = nullptr);

signals:
    void finished(bool success);

public slots:
    void process();

private:
    CodeGenerator *codeGenerator;
    bool success;
};

#endif // CODEWORKER_H
