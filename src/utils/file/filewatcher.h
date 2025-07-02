#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QString>

class FileWatcher : public QObject {
    Q_OBJECT

public:
    explicit FileWatcher(QObject *parent = nullptr);
    void watchProjectFiles(const QString &projectPath);
    QString getLastModifiedFile(); // Nuevo método
    std::string getLastModifiedFileInFolder(const std::string &folderPath);

signals:
    void fileChanged(const QString &filePath);

private:
    QFileSystemWatcher *watcher;
    QString lastModifiedFile;

};

#endif // FILEWATCHER_H
