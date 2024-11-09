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
    QString getVersionName() const;

private slots:
    void onRegisterButtonClicked(); // Slot para el botón de registro

private:
    Ui::CreateVersion *ui;
    VersionManager *versionManager; // Puntero a VersionManager para interactuar con el repositorio
};

#endif // CREATEVERSION_H
