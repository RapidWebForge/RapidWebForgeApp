#include "addfielddialog.h"
#include <QMessageBox>
#include "ui_addfielddialog.h"

AddFieldDialog::AddFieldDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddFieldDialog)
    , currentTransaction(nullptr)
{
    ui->setupUi(this);
}

AddFieldDialog::~AddFieldDialog()
{
    delete ui;
}

void AddFieldDialog::setTransaction(Transaction &transaction)
{
    currentTransaction = &transaction; // Asigna la referencia al Transaction actual
}

void AddFieldDialog::on_addButton_clicked()
{
    //if (currentTransaction == nullptr) {
    //    QMessageBox::warning(this, "Error", "Transaction is not set.");
    //    return;
    //}
    // Primero, comprobar si ya existe un campo marcado como Primary Key
    //bool primaryKeyExists = false;

    // Comprobar si ya existe un campo marcado como Primary Key
    //for (const Field &existingField : currentTransaction->getFields()) {
    //    if (existingField.isPrimaryKey()) {
    //        primaryKeyExists = true;
    //        break;
    //    }
    //}

    // Si ya existe una Primary Key, y el campo actual también tiene el checkbox de Primary Key activado, mostrar un mensaje de advertencia
    //if (primaryKeyExists && ui->primaryKeyCheckBox->isChecked()) {
    //    QMessageBox::warning(this,
    //                         "Invalid Operation",
    //                         "Only one Primary Key is allowed per table.");
    //    return;
    //}

    std::string fieldName = ui->fieldNameLineEdit->text().toStdString();
    field.setName(fieldName);

    std::string fieldType = ui->fieldTypeComboBox->currentText().toStdString();
    field.setType(fieldType);

    //field.setIsPrimaryKey(ui->primaryKeyCheckBox->isChecked());

    std::string constraint = ui->constraintComboBox->currentData().toString().toStdString();
    if (constraint == "NULL") {
        field.setIsNull(true);
        field.setIsUnique(false);
    } else {
        field.setIsNull(false);
        field.setIsUnique(true);
    }

    emit fieldSaved(field);

    // Limpiar el formulario
    ui->fieldNameLineEdit->clear();            // Limpiar el input del nombre del campo
    ui->fieldTypeComboBox->setCurrentIndex(0); // Resetear el combo box al primer valor
    ui->primaryKeyCheckBox->setChecked(false); // Desmarcar el checkbox de llave primaria
    ui->foreignKeyCheckBox->setChecked(false); // Desmarcar el checkbox de llave foránea
    ui->constraintComboBox->setCurrentIndex(
        0); // Resetear el combo box de constraint al primer valor

    accept();
}
