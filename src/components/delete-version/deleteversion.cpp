#include "deleteversion.h"
#include "ui_deleteversion.h"

DeleteVersion::DeleteVersion(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DeleteVersion)
    , model(new QStandardItemModel(this))
{
    ui->setupUi(this);

    // Configurar el modelo para el QListView
    ui->versionsListView->setModel(model);

    // Conectar el botón de "Delete" para aceptar el diálogo
    connect(ui->deleteButton, &QPushButton::clicked, this, &QDialog::accept);

    // Conectar el botón de "Cancel" para cerrar el diálogo sin cambios
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

DeleteVersion::~DeleteVersion()
{
    delete ui;
}

void DeleteVersion::setVersions(const std::vector<std::string> &versions)
{
    model->clear(); // Limpiar el modelo antes de añadir versiones nuevas

    // Agregar las versiones al modelo
    for (const auto &version : versions) {
        QStandardItem *item = new QStandardItem(QString::fromStdString(version));
        model->appendRow(item);
    }
}

QString DeleteVersion::getSelectedVersion() const
{
    QModelIndexList selectedIndexes = ui->versionsListView->selectionModel()->selectedIndexes();
    if (!selectedIndexes.isEmpty()) {
        return selectedIndexes.first().data().toString();
    }
    return QString();
}
