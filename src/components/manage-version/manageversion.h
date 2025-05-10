#ifndef MANAGEVERSION_H
#define MANAGEVERSION_H

#include <QDialog>
#include <QStandardItemModel>
#include <QString>
#include "../../core/version-manager/versionmanager.h"

namespace Ui {
class ManageVersion;
}
class ManageVersion : public QDialog
{
    Q_OBJECT

public:
    explicit ManageVersion(VersionManager *versionManager, QWidget *parent = nullptr);
    ~ManageVersion();

private slots:
    void on_acceptButton_clicked();

private:
    Ui::ManageVersion *ui;
    VersionManager *versionManager;
    QStandardItemModel *model; // Modelo para QListView

    void applyStyles();
};

#endif // MANAGEVERSION_H
