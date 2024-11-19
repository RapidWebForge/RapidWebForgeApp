#include "editfielddialog.h"
#include "ui_editfielddialog.h"

EditFieldDialog::EditFieldDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EditFieldDialog)
{
    ui->setupUi(this);

    //Ocultar defaul language
    ui->primaryKeyLabel->hide();
    ui->primaryKeyCheckBox->hide();
}

EditFieldDialog::~EditFieldDialog()
{
    delete ui;
}

void EditFieldDialog::setField(const Field &field)
{
    currentField = field;

    // Cargar los datos del field en el diálogo
    ui->fieldNameLineEdit->setText(QString::fromStdString(field.getName()));
    ui->fieldTypeComboBox->setCurrentText(QString::fromStdString(field.getType()));
    ui->primaryKeyCheckBox->setChecked(field.isPrimaryKey());
    ui->foreignKeyCheckBox->setChecked(field.isForeignKey());
    ui->nullCheckBox->setChecked(field.getIsNull());
    ui->uniqueCheckBox->setChecked(field.getIsUnique());
    ui->checkCheckBox->setChecked(field.getHasCheck());
    ui->defaultCheckBox->setChecked(field.getHasDefault());

    // Si es Foreign Key, mostrar la tabla relacionada
    if (field.isForeignKey()) {
        ui->foreignKeyTableComboBox->setEnabled(true);
        ui->foreignKeyTableComboBox->setCurrentText(
            QString::fromStdString(field.getForeignKeyTable()));
    } else {
        ui->foreignKeyTableComboBox->setEnabled(false);
    }
}

Field EditFieldDialog::getField() const
{
    // Actualizar los datos del campo actual desde el diálogo
    Field updatedField = currentField;
    updatedField.setName(ui->fieldNameLineEdit->text().toStdString());
    updatedField.setType(ui->fieldTypeComboBox->currentText().toStdString());
    updatedField.setIsPrimaryKey(ui->primaryKeyCheckBox->isChecked());
    updatedField.setIsForeignKey(ui->foreignKeyCheckBox->isChecked());
    // Completar el resto de los datos, como los constraints
    updatedField.setIsNull(ui->nullCheckBox->isChecked());
    updatedField.setIsUnique(ui->uniqueCheckBox->isChecked());

    // Si el campo es Foreign Key, asignar la tabla relacionada
    if (updatedField.isForeignKey()) {
        updatedField.setForeignKeyTable(ui->foreignKeyTableComboBox->currentText().toStdString());
    }

    return updatedField;
}

void EditFieldDialog::on_acceptButton_clicked()
{
    // Actualizar los valores del field actual con los datos del diálogo
    currentField.setName(ui->fieldNameLineEdit->text().toStdString());
    currentField.setType(ui->fieldTypeComboBox->currentText().toStdString());
    currentField.setIsPrimaryKey(ui->primaryKeyCheckBox->isChecked());
    currentField.setIsForeignKey(ui->foreignKeyCheckBox->isChecked());
    currentField.setIsNull(ui->nullCheckBox->isChecked());
    currentField.setIsUnique(ui->uniqueCheckBox->isChecked());
    currentField.setHasCheck(ui->checkCheckBox->isChecked());
    currentField.setHasDefault(ui->defaultCheckBox->isChecked());

    if (ui->foreignKeyCheckBox->isChecked()) {
        currentField.setForeignKeyTable(ui->foreignKeyTableComboBox->currentText().toStdString());
    }

    // Emitir la señal con los datos actualizados
    emit fieldSaved(getField()); // Aquí utilizamos getField() en lugar de getFieldData()

    // Aceptar el diálogo y cerrar
    accept();
}
