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

void EditFieldDialog::setAvailableTables(const std::vector<QString> &tables,
                                         const QString &currentTableName)
{
    // Limpiar el combo box antes de añadir nuevas tablas
    ui->foreignKeyTableComboBox->clear();

    // Añadir los nombres de las tablas al combo box
    for (const auto &table : tables) {
        if (table != currentTableName) { // Filtrar la tabla actual
            ui->foreignKeyTableComboBox->addItem(table);
        }
    }
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

void EditFieldDialog::on_foreignKeyCheckBox_stateChanged(int state)
{
    if (state == Qt::Checked) {
        // Si el checkbox está marcado, habilitar el combo box
        ui->foreignKeyTableComboBox->setEnabled(true);
        ui->nullCheckBox->setChecked(false);
        ui->uniqueCheckBox->setChecked(false);
        ui->nullCheckBox->setEnabled(false);
        ui->uniqueCheckBox->setEnabled(false);
        ui->fieldNameLineEdit->setText(ui->foreignKeyTableComboBox->currentText().toLower()
                                       + QString::fromStdString("Id"));
        ui->fieldTypeComboBox->setCurrentIndex(3); // INTEGER
        ui->fieldTypeComboBox->setEnabled(false);
        ui->fieldNameLineEdit->setEnabled(false);
    } else {
        // Si el checkbox no está marcado, deshabilitar el combo box
        ui->foreignKeyTableComboBox->setEnabled(false);
        ui->nullCheckBox->setEnabled(true);
        ui->uniqueCheckBox->setEnabled(true);
        ui->fieldTypeComboBox->setCurrentIndex(0);
        ui->fieldTypeComboBox->setEnabled(true);
        ui->fieldNameLineEdit->setEnabled(true);
        ui->fieldNameLineEdit->clear();
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
