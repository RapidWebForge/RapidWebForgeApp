#include "addfielddialog.h"
#include "ui_addfielddialog.h"

AddFieldDialog::AddFieldDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddFieldDialog)
{
    ui->setupUi(this);
}

AddFieldDialog::~AddFieldDialog()
{
    delete ui;
}

void AddFieldDialog::on_addButton_clicked()
{
    std::string fieldName = ui->fieldNameLineEdit->text().toStdString();
    field.setName(fieldName);

    std::string fieldType = ui->fieldTypeComboBox->currentText().toStdString();
    field.setType(fieldType);

    std::string constraint = ui->constraintComboBox->currentData().toString().toStdString();
    if (constraint == "NULL") {
        field.setIsNull(true);
        field.setIsUnique(false);
    } else {
        field.setIsNull(false);
        field.setIsUnique(true);
    }

    emit fieldSaved(field);

    accept();
}
