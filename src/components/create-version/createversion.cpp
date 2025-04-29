#include "createversion.h"
#include <QFile>
#include <QMessageBox>
#include "ui_createversion.h"

CreateVersion::CreateVersion(VersionManager *versionManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateVersion)
    , versionManager(versionManager)
{
    ui->setupUi(this);

    connect(ui->cancelButton, &QPushButton::clicked, this, &CreateVersion::reject);

    applyStyles();
}

CreateVersion::~CreateVersion()
{
    delete ui;
}

void CreateVersion::applyStyles()
{
    QFile primaryButtonstyleFile(":/styles/primarybutton");
    if (primaryButtonstyleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(primaryButtonstyleFile.readAll());
        ui->registerButton->setStyleSheet(styleSheet);
    }
    QFile secondaryButtonstyleFile(":/styles/secondarybutton");
    if (secondaryButtonstyleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(secondaryButtonstyleFile.readAll());
        ui->cancelButton->setStyleSheet(styleSheet);
    }
}

void CreateVersion::on_registerButton_clicked()
{
    // Obtener el nombre de la versión y los comentarios del usuario
    QString versionName = ui->versionNameLineEdit->text();

    if (versionName.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Version name cannot be empty.");
        return;
    }

    // Llamar a VersionManager para crear la versión
    versionManager->createVersion(versionName.toStdString());

    // Cerrar el diálogo
    accept();
}
