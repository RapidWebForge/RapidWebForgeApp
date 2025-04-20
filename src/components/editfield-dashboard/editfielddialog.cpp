#include "editfielddialog.h"
#include "ui_editfielddialog.h"

EditFieldDialog::EditFieldDialog(QWidget *parent)
    : QDialog(parent)
    , currentField(nullptr)
    , ui(new Ui::EditFieldDialog)
{
    ui->setupUi(this);

    ui->primaryKeyLabel->hide();
    ui->primaryKeyCheckBox->hide();
}

EditFieldDialog::~EditFieldDialog()
{
    delete ui;
}

void EditFieldDialog::setField(Field *field)
{
    currentField = field;
    setUpWidget(); // repoblar los widgets con el nuevo field
}

void EditFieldDialog::setUpWidget()
{
    // Cargar los datos del field en el diálogo
    ui->fieldNameLineEdit->setText(QString::fromStdString(currentField->getName()));

    QString typeToSelect = QString::fromStdString(currentField->getType());

    for (int i = 0; i < ui->fieldTypeComboBox->count(); ++i) {
        if (ui->fieldTypeComboBox->itemText(i) == typeToSelect) {
            ui->fieldTypeComboBox->setCurrentIndex(i);
            break;
        }
    }

    ui->primaryKeyCheckBox->setChecked(currentField->isPrimaryKey());
    ui->foreignKeyCheckBox->setChecked(currentField->isForeignKey());
    ui->nullCheckBox->setChecked(currentField->getIsNull());
    ui->uniqueCheckBox->setChecked(currentField->getIsUnique());
    // ui->checkCheckBox->setChecked(currentField->getHasCheck());
    // ui->defaultCheckBox->setChecked(currentField->getHasDefault());
    ui->checkCheckBox->setHidden(true);
    ui->defaultCheckBox->setHidden(true);

    // Si es Foreign Key, mostrar la tabla relacionada
    if (currentField->isForeignKey()) {
        ui->foreignKeyTableComboBox->setEnabled(true);
        ui->foreignKeyTableComboBox->setCurrentText(
            QString::fromStdString(currentField->getForeignKeyTable()));
    } else {
        ui->foreignKeyTableComboBox->setEnabled(false);
    }
}

void EditFieldDialog::on_acceptButton_clicked()
{
    // Actualizar los valores del field actual con los datos del diálogo
    currentField->setName(ui->fieldNameLineEdit->text().toStdString());
    currentField->setType(ui->fieldTypeComboBox->currentText().toStdString());
    currentField->setIsPrimaryKey(ui->primaryKeyCheckBox->isChecked());
    currentField->setIsForeignKey(ui->foreignKeyCheckBox->isChecked());
    currentField->setIsNull(ui->nullCheckBox->isChecked());
    currentField->setIsUnique(ui->uniqueCheckBox->isChecked());
    // currentField->setHasCheck(ui->checkCheckBox->isChecked());
    // currentField->setHasDefault(ui->defaultCheckBox->isChecked());

    if (ui->foreignKeyCheckBox->isChecked()) {
        currentField->setForeignKeyTable(ui->foreignKeyTableComboBox->currentText().toStdString());
    }

    // Emitir la señal con los datos actualizados
    emit fieldSaved();

    // Aceptar el diálogo y cerrar
    accept();
}
