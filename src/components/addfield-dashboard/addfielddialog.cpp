#include "addfielddialog.h"
#include <QMessageBox>
#include "ui_addfielddialog.h"

AddFieldDialog::AddFieldDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddFieldDialog)
    , currentTransaction(nullptr)
{
    ui->setupUi(this);

    // Deshabilitar el combo box al inicio
    ui->foreignKeyTableComboBox->setEnabled(false);
    // Conectar el evento del checkbox para habilitar o deshabilitar el combo box
    connect(ui->foreignKeyCheckBox,
            &QCheckBox::checkStateChanged, // Cambiar stateChanged a checkStateChanged
            this,
            &AddFieldDialog::on_foreignKeyCheckBox_stateChanged);

    //Ocultar defaul language
    ui->primaryKeyLabel->hide();
    ui->primaryKeyCheckBox->hide();
}

AddFieldDialog::~AddFieldDialog()
{
    delete ui;
}

void AddFieldDialog::setTransaction(Transaction &transaction)
{
    currentTransaction = &transaction; // Asigna la referencia al Transaction actual

    // Verificar si ya existe un campo con Primary Key
    bool primaryKeyExists = false;
    for (const Field &existingField : currentTransaction->getFields()) {
        if (existingField.isPrimaryKey()) {
            primaryKeyExists = true;
            break;
        }
    }

    // Si ya existe una Primary Key, deshabilitar el checkbox de Primary Key
    if (primaryKeyExists) {
        ui->primaryKeyCheckBox->setEnabled(false);
    } else {
        ui->primaryKeyCheckBox->setEnabled(true);
    }
}

void AddFieldDialog::setAvailableTables(const std::vector<QString> &tables,
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

void AddFieldDialog::on_addButton_clicked()
{
    if (currentTransaction == nullptr) {
        QMessageBox::warning(this, "Error", "Transaction is not set.");
        return;
    }
    // Primero, comprobar si ya existe un campo marcado como Primary Key
    bool primaryKeyExists = false;

    // Comprobar si ya existe un campo marcado como Primary Key
    for (const Field &existingField : currentTransaction->getFields()) {
        if (existingField.isPrimaryKey()) {
            primaryKeyExists = true;
            break;
        }
    }

    // Si ya existe una Primary Key, y el campo actual también tiene el checkbox de Primary Key activado, mostrar un mensaje de advertencia
    if (primaryKeyExists && ui->primaryKeyCheckBox->isChecked()) {
        QMessageBox::warning(this,
                             "Invalid Operation",
                             "Only one Primary Key is allowed per table.");
        return;
    }

    // Crear el nuevo campo
    Field field;
    std::string fieldName = ui->fieldNameLineEdit->text().toStdString();
    field.setName(fieldName);

    std::string fieldType = ui->fieldTypeComboBox->currentText().toStdString();
    field.setType(fieldType);

    // Verificar las restricciones de NULL y UNIQUE
    bool isNull = ui->nullCheckBox->isChecked();
    bool isUnique = ui->uniqueCheckBox->isChecked();
    bool hasCheck = ui->checkCheckBox->isChecked();
    bool hasDefault = ui->defaultCheckBox->isChecked();

    field.setIsPrimaryKey(ui->primaryKeyCheckBox->isChecked());
    field.setIsNull(isNull);
    field.setIsUnique(isUnique);
    field.setHasCheck(hasCheck);
    field.setHasDefault(hasDefault);

    if (ui->foreignKeyCheckBox->isChecked()) {
        std::string foreignKeyTable = ui->foreignKeyTableComboBox->currentText().toStdString();
        field.setIsForeignKey(true);
        field.setForeignKeyTable(foreignKeyTable); // Establecer la tabla relacionada
    }
    emit fieldSaved(field);

    // Limpiar el formulario
    ui->fieldNameLineEdit->clear();
    ui->fieldTypeComboBox->setCurrentIndex(0);
    ui->primaryKeyCheckBox->setChecked(false);
    ui->foreignKeyCheckBox->setChecked(false);
    ui->nullCheckBox->setChecked(false);
    ui->uniqueCheckBox->setChecked(false);
    ui->checkCheckBox->setChecked(false);
    ui->defaultCheckBox->setChecked(false);
    accept();
}

void AddFieldDialog::on_foreignKeyCheckBox_stateChanged(int state)
{
    if (state == Qt::Checked) {
        // Si el checkbox está marcado, habilitar el combo box
        ui->foreignKeyTableComboBox->setEnabled(true);
    } else {
        // Si el checkbox no está marcado, deshabilitar el combo box
        ui->foreignKeyTableComboBox->setEnabled(false);
    }
}
