#ifndef DELETEVERSION_H
#define DELETEVERSION_H

#include <QDialog>
#include <QStandardItemModel>
#include "../../core/version-manager/versionmanager.h"

namespace Ui {
class DeleteVersion;
}

class DeleteVersion : public QDialog
{
    Q_OBJECT

public:
    explicit DeleteVersion(VersionManager *versionManager, QWidget *parent = nullptr);
    ~DeleteVersion();

private slots:
    void on_deleteButton_clicked();

private:
    Ui::DeleteVersion *ui;
    VersionManager *versionManager;
    QStandardItemModel *model;

    void applyStyles();
};

#endif // DELETEVERSION_H
