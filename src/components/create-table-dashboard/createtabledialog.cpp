#include "createtabledialog.h"
#include "ui_createtabledialog.h"
#include <boost/algorithm/string.hpp>

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

        connect(addFieldDialog, &AddFieldDialog::fieldSaved, this, &CreateTableDialog::onFieldSaved);
    }
    addFieldDialog->exec();
}

void CreateTableDialog::onFieldSaved(const Field &field)
{
    transaction.getFields().push_back(field);
}

void CreateTableDialog::on_createButton_clicked()
{
    std::string transactionName = ui->tableNameLineEdit->text().toStdString();
    // TODO: Ensure capitalize
    transaction.setName(transactionName);
    transaction.setNameConst(boost::to_lower_copy(transactionName));

    emit transactionSaved(transaction);

    // Limpiar el campo de texto después de crear la transacción
    ui->tableNameLineEdit->clear(); // Esto limpia el input de la tabla
}
