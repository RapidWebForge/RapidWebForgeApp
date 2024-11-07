#include "createversion.h"
#include <QMessageBox>
#include "ui_createversion.h"

CreateVersion::CreateVersion(VersionManager *versionManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateVersion)
    , versionManager(versionManager)
{
    ui->setupUi(this);

    // Conectar el botón "Registrar" al slot para aceptar el diálogo
    connect(ui->registerButton, &QPushButton::clicked, this, &CreateVersion::accept);
    connect(ui->cancelButton,
            &QPushButton::clicked,
            this,
            &CreateVersion::reject); // Cerrar el diálogo al cancelar
}

CreateVersion::~CreateVersion()
{
    delete ui;
}

void CreateVersion::onRegisterButtonClicked()
{
    // Obtener el nombre de la versión y los comentarios del usuario
    QString versionName = ui->versionNameLineEdit->text();
    QString comments = ui->commentsTextEdit->toPlainText();

    if (versionName.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Version name cannot be empty.");
        return;
    }

    // Llamar a VersionManager para crear la versión
    versionManager->createVersion(versionName.toStdString());

    // Mostrar un mensaje de éxito
    QMessageBox::information(this, "Success", "Version created successfully.");

    // Cerrar el diálogo
    accept();
}
// Método para obtener el nombre de la versión ingresado por el usuario
QString CreateVersion::getVersionName() const
{
    return ui->versionNameLineEdit->text();
}
