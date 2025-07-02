#include "deleteversion.h"
#include <QFile>
#include <QMessageBox>
#include "ui_deleteversion.h"

DeleteVersion::DeleteVersion(VersionManager *versionManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DeleteVersion)
    , versionManager(versionManager)
    , model(new QStandardItemModel(this))
{
    ui->setupUi(this);

    // Conectar el botón de "Cancel" para cerrar el diálogo sin cambios
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    model->clear();

    // Agregar las versiones al modelo
    std::vector<std::string> versions = versionManager->listVersions();
    for (const auto &version : versions) {
        QStandardItem *item = new QStandardItem(QString::fromStdString(version));
        model->appendRow(item);
    }

    // Configurar el modelo para el QListView
    ui->versionsListView->setModel(model);

    applyStyles();
}

DeleteVersion::~DeleteVersion()
{
    delete ui;
}

void DeleteVersion::applyStyles()
{
    QFile primaryButtonstyleFile(":/styles/deletebutton");
    if (primaryButtonstyleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(primaryButtonstyleFile.readAll());
        ui->deleteButton->setStyleSheet(styleSheet);
    }
    QFile secondaryButtonstyleFile(":/styles/secondarybutton");
    if (secondaryButtonstyleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(secondaryButtonstyleFile.readAll());
        ui->cancelButton->setStyleSheet(styleSheet);
    }
}

void DeleteVersion::on_deleteButton_clicked()
{
    QString selectedVersion;

    QMessageBox msgBox;
    msgBox.setStyleSheet(
        "QPushButton { background-color: #f0f0f0; color: black; padding: 5px 10px; }"
        "QMessageBox { background-color: white; }");

    msgBox.setWindowTitle("Delete Version");
    msgBox.setText("Are you sure you want to delete the selected version?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    int reply = msgBox.exec();

    if (reply == QMessageBox::No)
        return;

    QModelIndexList selectedIndexes = ui->versionsListView->selectionModel()->selectedIndexes();
    if (!selectedIndexes.isEmpty()) {
        selectedVersion = selectedIndexes.first().data().toString();
    }

    if (selectedVersion.isEmpty())
        return;

    // Llamar a VersionManager para crear la versión
    versionManager->deleteVersion(selectedVersion.toStdString());

    // Cerrar el diálogo
    accept();
}
