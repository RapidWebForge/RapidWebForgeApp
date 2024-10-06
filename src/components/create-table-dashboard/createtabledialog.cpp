#include "createtabledialog.h"
#include "ui_createtabledialog.h"

CreateTableDialog::CreateTableDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateTableDialog)
    , addFieldDialog(nullptr)
{
    ui->setupUi(this);
    connect(ui->addFieldButton, &QPushButton::clicked, this, &CreateTableDialog::showAddFieldDialog);
}

CreateTableDialog::~CreateTableDialog()
{
    delete ui;
}
void CreateTableDialog::showAddFieldDialog()
{
    if (!addFieldDialog) {
        addFieldDialog = new AddFieldDialog(this);
    }
    addFieldDialog->exec(); // Muestra el diálogo de forma modal
}
