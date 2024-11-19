#include "versionhistory.h"
#include <QFile>
#include "ui_versionhistory.h"

VersionHistory::VersionHistory(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::VersionHistory)
{
    ui->setupUi(this);

    // Conectar el botón de cerrar
    connect(ui->closeButton, &QPushButton::clicked, this, &QDialog::accept);

    applyStyles();
}

VersionHistory::~VersionHistory()
{
    delete ui;
}

void VersionHistory::applyStyles()
{
    QFile primaryButtonstyleFile(":/styles/primarybutton");
    if (primaryButtonstyleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(primaryButtonstyleFile.readAll());
        ui->closeButton->setStyleSheet(styleSheet);
    }
}

void VersionHistory::setVersions(const std::vector<std::string> &versions)
{
    ui->listWidget->clear(); // Asegúrate de que la lista esté vacía al inicio

    for (const std::string &version : versions) {
        ui->listWidget->addItem(QString::fromStdString(version)); // Añadir cada versión
    }
}
void VersionHistory::setCommits(const std::vector<std::string> &commits)
{
    ui->listWidget->clear(); // Limpiar `listWidget` antes de agregar los commits

    for (const std::string &commit : commits) {
        ui->listWidget->addItem(QString::fromStdString(commit)); // Añadir cada commit
    }
}
void VersionHistory::setBranches(const std::vector<std::string> &branches)
{
    ui->listWidget->clear(); // Limpiar `listWidget` antes de agregar las ramas

    for (const auto &branch : branches) {
        ui->listWidget->addItem(QString::fromStdString(branch)); // Añadir cada rama
    }
}
