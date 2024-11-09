#include "manageversion.h"
#include <QFile>
#include "../../core/version-manager/versionmanager.h"
#include "ui_manageversion.h"

ManageVersion::ManageVersion(VersionManager *versionManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ManageVersion)
    , versionManager(versionManager)
    , model(new QStandardItemModel(this)) // Inicializar el modelo para QListView

{
    ui->setupUi(this);

    // Conectar el botón "Change to" para aceptar el diálogo
    connect(ui->acceptButton, &QPushButton::clicked, this, &QDialog::accept);

    // Cargar ramas en el QListView
    std::vector<std::string> branches = versionManager->listVersions();
    for (const std::string &branch : branches) {
        QStandardItem *item = new QStandardItem(QString::fromStdString(branch));
        model->appendRow(item);
    }

    ui->versionslistView->setModel(model);

    applyStyles();
}

ManageVersion::~ManageVersion()
{
    delete ui;
}

void ManageVersion::applyStyles()
{
    QFile primaryButtonstyleFile(":/styles/primarybutton");
    if (primaryButtonstyleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(primaryButtonstyleFile.readAll());
        ui->acceptButton->setStyleSheet(styleSheet);
    }
    QFile secondaryButtonstyleFile(":/styles/secondarybutton");
    if (secondaryButtonstyleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(secondaryButtonstyleFile.readAll());
        ui->cancelButton->setStyleSheet(styleSheet);
    }
}

// Método para obtener la rama seleccionada en versionslistView
QString ManageVersion::getSelectedBranch() const
{
    QModelIndexList selectedIndexes = ui->versionslistView->selectionModel()->selectedIndexes();
    if (!selectedIndexes.isEmpty()) {
        return selectedIndexes.first().data().toString();
    }
    return QString();
}
