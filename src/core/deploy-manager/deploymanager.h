#ifndef DEPLOYMANAGER_H
#define DEPLOYMANAGER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <string>

class DeployManager : public QObject
{
    Q_OBJECT

public:
    DeployManager(const std::string &projectPath, QObject *parent = nullptr);
    DeployManager(QObject *parent = nullptr);
    ~DeployManager();

    void start();
    void kill();

private:
    std::string projectPath;
    std::string ngInxPath;
    std::string bunPath;

    QString configFilePath;
    QString ngInxDirectory;

    // QProcess *backPortProcess = nullptr;
    // QProcess *frontPortProcess = nullptr;
    QProcess *backProcess = nullptr;
    QProcess *frontProcess = nullptr;
    QProcess *nginxProcess = nullptr;

    void createNginxConfig(int frontendPort, int backendPort);
    bool spawnServer(const QString &dir, quint16 port, QProcess *&handle);
    void spawnTerminal(const QString &dir);
};

#endif // DEPLOYMANAGER_H
