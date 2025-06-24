#include "addfielddialog.h"
#include <QFile>
#include <QMessageBox>
#include "ui_addfielddialog.h"
#include <algorithm>
#include <cctype>
#include <string>

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
            &QCheckBox::checkStateChanged,
            this,
            &AddFieldDialog::on_foreignKeyCheckBox_stateChanged);

    ui->primaryKeyLabel->hide();
    ui->primaryKeyCheckBox->hide();
    ui->checkCheckBox->setHidden(true);
    ui->defaultCheckBox->setHidden(true);

    applyStyles();
}

AddFieldDialog::~AddFieldDialog()
{
    delete ui;
}

void AddFieldDialog::applyStyles()
{
    QFile primaryButtonstyleFile(":/styles/primarybutton");
    if (primaryButtonstyleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(primaryButtonstyleFile.readAll());
        ui->addButton->setStyleSheet(styleSheet);
    }
    QFile secondaryButtonstyleFile(":/styles/secondarybutton");
    if (secondaryButtonstyleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(secondaryButtonstyleFile.readAll());
        ui->cancelButton->setStyleSheet(styleSheet);
    }
}

void AddFieldDialog::setTransaction(Transaction *transaction)
{
    currentTransaction = transaction; // Asigna la referencia al Transaction actual

    // Verificar si ya existe un campo con Primary Key
    // bool primaryKeyExists = false;
    // for (const Field &existingField : currentTransaction->getFields()) {
    //     if (existingField.isPrimaryKey()) {
    //         primaryKeyExists = true;
    //         break;
    //     }
    // }

    // // Si ya existe una Primary Key, deshabilitar el checkbox de Primary Key
    // if (primaryKeyExists) {
    //     ui->primaryKeyCheckBox->setEnabled(false);
    // } else {
    //     ui->primaryKeyCheckBox->setEnabled(true);
    // }
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

std::string normalizeString(const std::string &input)
{
    std::string result;

    // Convertir a minúsculas y eliminar espacios
    for (char c : input) {
        if (!std::isspace(static_cast<unsigned char>(c))) {        // Ignorar espacios
            result += std::tolower(static_cast<unsigned char>(c)); // Convertir a minúsculas
        }
    }

    return result;
}

bool areStringsEqual(const std::string &a, const std::string &b)
{
    return normalizeString(a) == normalizeString(b);
}

void AddFieldDialog::clearContent()
{
    // Limpiar el formulario
    ui->fieldNameLineEdit->clear();
    ui->fieldTypeComboBox->setCurrentIndex(0);
    ui->primaryKeyCheckBox->setChecked(false);
    ui->foreignKeyCheckBox->setChecked(false);
    ui->nullCheckBox->setChecked(false);
    ui->uniqueCheckBox->setChecked(false);
    // ui->checkCheckBox->setChecked(false);
    // ui->defaultCheckBox->setChecked(false);
}

void AddFieldDialog::on_addButton_clicked()
{
    if (currentTransaction == nullptr) {
        QMessageBox::warning(this, "Error", "Transaction is not set.");
        return;
    }
    // Primero, comprobar si ya existe un campo marcado como Primary Key
    // bool primaryKeyExists = false;

    // Comprobar si ya existe un campo marcado como Primary Key
    // for (const Field &existingField : currentTransaction->getFields()) {
    //     if (existingField.isPrimaryKey()) {
    //         primaryKeyExists = true;
    //         break;
    //     }
    // }

    // Si ya existe una Primary Key, y el campo actual también tiene el checkbox de Primary Key activado,
    // mostrar un mensaje de advertencia
    // if (primaryKeyExists && ui->primaryKeyCheckBox->isChecked()) {
    //     QMessageBox::warning(this,
    //                          "Invalid Operation",
    //                          "Only one Primary Key is allowed per table.");
    //     return;
    // }

    // Crear el nuevo campo
    Field field;
    std::string fieldName = ui->fieldNameLineEdit->text().toStdString();

    QRegularExpression invalidChars(R"([\\/:*?"<>|])");
    if (invalidChars.match(QString::fromStdString(fieldName)).hasMatch()) {
        QMessageBox::warning(this, "Invalid name", "Field name have invalid characters.");
        return;
    }
    if (fieldName.empty()) {
        QMessageBox::warning(this, "Error", "Field name cannot be empty.");
        return;
    }

    bool isCreatedBefore = false;

    for (auto field : currentTransaction->getFields()) {
        isCreatedBefore = areStringsEqual(field.getName(), fieldName);
        if (isCreatedBefore)
            break;
    }

    if (isCreatedBefore) {
        QMessageBox::critical(this,
                              "Warning",
                              "Try with another name, that was used in another field");
    } else {
        field.setName(fieldName);

        std::string fieldType = ui->fieldTypeComboBox->currentText().toStdString();
        field.setType(fieldType);

        // Verificar las restricciones de NULL y UNIQUE
        bool isNull = ui->nullCheckBox->isChecked();
        bool isUnique = ui->uniqueCheckBox->isChecked();
        // bool hasCheck = ui->checkCheckBox->isChecked();
        // bool hasDefault = ui->defaultCheckBox->isChecked();

        // field.setIsPrimaryKey(ui->primaryKeyCheckBox->isChecked());
        field.setIsPrimaryKey(false);
        field.setIsNull(isNull);
        field.setIsUnique(isUnique);
        // field.setHasCheck(hasCheck);
        // field.setHasDefault(hasDefault);

        std::string logMessage = "fieldName=" + fieldName + ", fieldType=" + fieldType
                                 + ", isPrimaryKey=" + (field.isPrimaryKey() ? "true" : "false")
                                 + ", isNull=" + (isNull ? "true" : "false")
                                 + ", isUnique=" + (isUnique ? "true" : "false");

        if (ui->foreignKeyCheckBox->isChecked()) {
            std::string foreignKeyTable = ui->foreignKeyTableComboBox->currentText().toStdString();
            field.setIsForeignKey(true);
            field.setForeignKeyTable(foreignKeyTable); // Establecer la tabla relacionada

            // Log de creación de relación entre modelos
            std::string relationshipLog = "sourceModel=" + currentTransaction->getName()
                                          + ", targetModel=" + foreignKeyTable
                                          + ", fieldName=" + fieldName;

            loggerJson.logAction("create-relationship-between-models", relationshipLog);

            logMessage += ", foreignKeyTable=" + foreignKeyTable;
        }

        if (fieldName.empty()) {
            QMessageBox::warning(this, "Warning", "Field name cannot be empty.");
            return;
        }

        // Log de la creación del nuevo field
        loggerJson.logAction("add-field-to-model", logMessage);
        currentTransaction->addField(field);
        emit fieldSaved();

        clearContent();

        accept();
    }
}

void AddFieldDialog::on_foreignKeyCheckBox_stateChanged(int state)
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

void AddFieldDialog::on_cancelButton_clicked()
{
    clearContent();

    this->close();
}
