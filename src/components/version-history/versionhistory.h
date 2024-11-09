#ifndef VERSIONHISTORY_H
#define VERSIONHISTORY_H

#include <QDialog>
#include <QListWidget>
#include <string>
#include <vector>

namespace Ui {
class VersionHistory;
}

class VersionHistory : public QDialog
{
    Q_OBJECT

public:
    explicit VersionHistory(QWidget *parent = nullptr);
    ~VersionHistory();

    // Método para cargar la lista de versiones en el widget
    void setVersions(const std::vector<std::string> &versions);
    void setCommits(const std::vector<std::string> &commits);
    void setBranches(const std::vector<std::string> &branches);

private:
    Ui::VersionHistory *ui;
    QListWidget *branchesListWidget; // Definición de branchesListWidget para mostrar las ramas
};

#endif // VERSIONHISTORY_H
