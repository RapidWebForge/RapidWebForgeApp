#ifndef CREATEVERSION_H
#define CREATEVERSION_H

#include <QDialog>
#include "../../core/version-manager/versionmanager.h"

namespace Ui {
class CreateVersion;
}

class CreateVersion : public QDialog
{
    Q_OBJECT

public:
    explicit CreateVersion(VersionManager *versionManager, QWidget *parent = nullptr);
    ~CreateVersion();

private slots:
    void on_registerButton_clicked();

private:
    Ui::CreateVersion *ui;
    VersionManager *versionManager;

    void applyStyles();
};

#endif // CREATEVERSION_H
