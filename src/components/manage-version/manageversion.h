#ifndef MANAGEVERSION_H
#define MANAGEVERSION_H

#include <QDialog>
#include <QStandardItemModel> // Para usar con QListView
#include <QString>
namespace Ui {
class ManageVersion;
}
class VersionManager; // Forward declaration
class ManageVersion : public QDialog
{
    Q_OBJECT

public:
    explicit ManageVersion(VersionManager *versionManager, QWidget *parent = nullptr);
    ~ManageVersion();

    QString getSelectedBranch() const;

private:
    Ui::ManageVersion *ui;
    VersionManager *versionManager;
    QStandardItemModel *model; // Modelo para QListView
};

#endif // MANAGEVERSION_H
