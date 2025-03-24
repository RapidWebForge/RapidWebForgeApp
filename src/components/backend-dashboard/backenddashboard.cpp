#include "backenddashboard.h"
#include <QFile>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include "ui_backenddashboard.h"
#include <algorithm>
#include <cctype>

BackendDashboard::BackendDashboard(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BackendDashboard)
    , createTableDialog(nullptr)
    , addFieldDialog(nullptr)
    , editFieldDialog(nullptr)
    , rootItem(nullptr)
{
    ui->setupUi(this);

    // Crear y configurar root
    rootItem = new QTreeWidgetItem(ui->tablesTreeWidget);
    rootItem->setText(0, "Database tables");
    rootItem->setIcon(0, QIcon(":/icons/database.png"));

    ui->tablesTreeWidget->expandAll();

    connect(ui->tablesTreeWidget,
            &QTreeWidget::itemClicked,
            this,
            &BackendDashboard::onTableSelected);

    setupMethodsList();
    setupFieldsTable();
    applyStylesBack();
}

BackendDashboard::~BackendDashboard()
{
    delete ui;
    delete createTableDialog;
    delete addFieldDialog;
}

void BackendDashboard::applyStylesBack()
{
    QFile styleFile(":/styles/backenddashboard");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        this->setStyleSheet(styleSheet);
    }

    ui->titleLabel->setStyleSheet("font-size: 35px; color: #27292A; padding-top: 10px; "
                                  "padding-left: 40px; padding-bottom: 20px;");

    ui->tableLabel->setStyleSheet("font-size: 16px; color: #27292A; padding-top: 0px; "
                                  "padding-left: 10px; padding-bottom: 10px;");

    ui->fieldLabel->setStyleSheet("font-size: 16px; color: #27292A; padding-top: 0px; "
                                  "padding-left: 10px; padding-bottom: 10px;");

    ui->labelMethods->setStyleSheet("font-size: 16px; color: #27292A; padding-top: 0px; "
                                    "padding-left: 10px; padding-bottom: 10px;");

    // Ajustar los iconos y tamaño de los botones
    ui->editField->setIcon(QIcon(":/icons/edit.png"));
    ui->editField->setIconSize(QSize(16, 16));
    ui->editField->setToolTip("Edit Field");

    ui->addField->setIcon(QIcon(":/icons/add.png"));
    ui->addField->setIconSize(QSize(16, 16));
    ui->addField->setToolTip("Add Field");

    ui->deleteField->setIcon(QIcon(":/icons/delete.png"));
    ui->deleteField->setIconSize(QSize(16, 16));
    ui->deleteField->setToolTip("Delete Field");

    ui->createTable->setIcon(QIcon(":/icons/adddb.png"));
    ui->createTable->setIconSize(QSize(16, 16));
    ui->createTable->setToolTip("Create Table");

    ui->deleteTable->setIcon(QIcon(":/icons/delete.png"));
    ui->deleteTable->setIconSize(QSize(16, 16));
    ui->deleteTable->setToolTip("Delete Table");

    ui->editTable->setIcon(QIcon(":/icons/edit.png"));
    ui->editTable->setIconSize(QSize(16, 16));
    ui->editTable->setToolTip("Edit Table");
}

void BackendDashboard::setupFieldsTable()
{
    // Configurar columnas y filas
    ui->fieldsTableWidget->setColumnCount(5);
    QStringList headers;
    headers << "Field name" << "Type" << "PK" << "FK" << "Const";
    ui->fieldsTableWidget->setHorizontalHeaderLabels(headers);

    // Configurar la propiedad de ajuste de texto (WordWrap)
    ui->fieldsTableWidget->setWordWrap(true);

    // Configurar el tamaño de las celdas para ajustarse al contenido
    ui->fieldsTableWidget->resizeColumnsToContents();
    ui->fieldsTableWidget->resizeRowsToContents();

    // Ajustar el tamaño de las celdas para adaptarse al contenido automáticamente
    ui->fieldsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->fieldsTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Ajustes de estilo y visualización
    ui->fieldsTableWidget->horizontalHeader()->setStretchLastSection(
        true); // Última columna ajustada al ancho restante
    ui->fieldsTableWidget->verticalHeader()->setVisible(false); // Oculta el encabezado vertical
    ui->fieldsTableWidget->setSelectionBehavior(
        QAbstractItemView::SelectRows); // Selección por filas
    ui->fieldsTableWidget->setEditTriggers(QAbstractItemView::DoubleClicked
                                           | QAbstractItemView::SelectedClicked);

    // Ajustes de estilo
    ui->fieldsTableWidget->setStyleSheet("QTableWidget {"
                                         "   background-color: #ffffff;"
                                         "   border: 1px solid #dcdcdc;"
                                         "   border-radius: 8px;"
                                         "   font-size: 14px;"
                                         "   color: #333;"
                                         "} "
                                         "QTableWidget::item {"
                                         "   padding: 10px;"
                                         "} "
                                         "QTableWidget::item:selected {"
                                         "   background-color: #0F66DE;"
                                         "   color: white;"
                                         "}");

    // Establecer alineación para las celdas de las columnas de tipo PK y FK
    for (int row = 0; row < ui->fieldsTableWidget->rowCount(); ++row) {
        QTableWidgetItem *pkItem = new QTableWidgetItem();
        pkItem->setCheckState(Qt::Unchecked);
        pkItem->setTextAlignment(Qt::AlignCenter);
        ui->fieldsTableWidget->setItem(row, 2, pkItem);

        QTableWidgetItem *fkItem = new QTableWidgetItem();
        fkItem->setCheckState(Qt::Unchecked);
        fkItem->setTextAlignment(Qt::AlignCenter);
        ui->fieldsTableWidget->setItem(row, 3, fkItem);
    }

    // Ajustes de visualización
    ui->fieldsTableWidget->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);                                  // Extiende las columnas
    ui->fieldsTableWidget->verticalHeader()->setVisible(false); // Oculta el encabezado vertical
    ui->fieldsTableWidget->setSelectionBehavior(
        QAbstractItemView::SelectRows); // Selección por filas
}

void BackendDashboard::setupMethodsList()
{
    // Crear una lista de métodos
    QStringList methods = {"View Tasks", "Add Tasks", "Edit Tasks", "Delete Tasks"};

    // Crear el elemento "Method type" como el primer elemento sin QCheckBox
    QListWidgetItem *headerItem = new QListWidgetItem(ui->tasksMethodsListWidget);
    headerItem->setFlags(Qt::NoItemFlags); // Hacer que no se pueda seleccionar ni editar
    QWidget *headerContainer = new QWidget(ui->tasksMethodsListWidget);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerContainer);

    QLabel *methodTypeLabel = new QLabel("Method type", headerContainer);
    methodTypeLabel->setObjectName("methodTypeLabel");
    methodTypeLabel->setAlignment(Qt::AlignCenter);
    methodTypeLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #333;");

    headerLayout->addWidget(methodTypeLabel);
    headerLayout->setAlignment(Qt::AlignCenter);
    headerLayout->setContentsMargins(10, 10, 10, 10);
    headerContainer->setLayout(headerLayout);

    headerItem->setSizeHint(headerContainer->sizeHint());
    ui->tasksMethodsListWidget->addItem(headerItem);
    ui->tasksMethodsListWidget->setItemWidget(headerItem, headerContainer);

    // Crear y agregar un QCheckBox para cada método en el QListWidget
    for (const QString &method : methods) {
        QListWidgetItem *item = new QListWidgetItem(ui->tasksMethodsListWidget);
        QWidget *container = new QWidget(ui->tasksMethodsListWidget);
        QHBoxLayout *layout = new QHBoxLayout(container);

        // Crear un QCheckBox y agregarlo al layout del contenedor
        QCheckBox *checkbox = new QCheckBox(method, container);
        checkbox->setChecked(true); // Establecer como marcado por defecto
        checkbox->setEnabled(false); // No se permite cambiar los valores de los checkbox
        layout->addWidget(checkbox);
        layout->setAlignment(Qt::AlignLeft); // Alinear a la izquierda

        container->setLayout(layout);
        item->setSizeHint(container->sizeHint()); // Ajustar el tamaño del item al contenedor
        ui->tasksMethodsListWidget->addItem(item);
        ui->tasksMethodsListWidget->setItemWidget(item, container);
    }

    // Ajustar estilos para la lista y los elementos
    ui->tasksMethodsListWidget->setStyleSheet("QListWidget {"
                                              "   border-radius: 8px;"
                                              "   padding: 5px;"
                                              "   font-size: 14px;"
                                              "} "
                                              "QCheckBox {"
                                              "   spacing: 10px;"
                                              "   font-size: 16px;"
                                              "   color: #27292A;"
                                              "} "
                                              "QCheckBox::indicator {"
                                              "   width: 20px;"
                                              "   height: 20px;"
                                              "}");
}

// Setters
void BackendDashboard::setTransactions(const std::vector<Transaction> &newTransactions)
{
    transactions = newTransactions;

    // Limpiar rootItem
    rootItem->takeChildren();

    // Añadir transactions como hijos
    for (const auto &transaction : transactions) {
        QTreeWidgetItem *item = new QTreeWidgetItem(rootItem);
        item->setText(0, QString::fromStdString(transaction.getName()));
    }

    // Expandir todo el árbol para mostrar todas las tablas
    ui->tablesTreeWidget->expandAll();

    // Si transactions están habilitados, configurar el primero como la transaction actual
    if (!transactions.empty()) {
        // Cargar el primer transaction automáticamente
        setCurrentTransaction(transactions[0]);
        updateFieldsTable(transactions[0]);

        // Actualizar el UI de los labels para la primera transaction
        ui->fieldLabel->setText(QString::fromStdString(transactions[0].getName()) + " Table");
        ui->labelMethods->setText(QString::fromStdString(transactions[0].getName()) + " Methods");
    }
}

void BackendDashboard::setCurrentTransaction(Transaction &transaction)
{
    currentTransaction = transaction;
}

void BackendDashboard::onFieldSaved(const Field &field)
{
    // Buscar el campo existente y actualizarlo
    bool fieldUpdated = false;
    for (auto &existingField : currentTransaction.getFields()) {
        if (existingField.getName() == field.getName()) {
            existingField = field; // Actualizar el campo existente
            fieldUpdated = true;
            break;
        }
    }

    // Si no se encontró el campo, lo agregamos (esto no debería suceder durante una edición)
    if (!fieldUpdated) {
        qDebug() << "Adding new field: " << QString::fromStdString(field.getName());
        currentTransaction.getFields().push_back(field); // Agregar el nuevo campo
    }

    // Actualizar la transacción en la lista de transacciones
    for (auto &transaction : transactions) {
        if (transaction.getName() == currentTransaction.getName()) {
            transaction.setFields(currentTransaction.getFields());
            break;
        }
    }
    updateFieldsTable(currentTransaction);
}

void BackendDashboard::onTransactionSaved(const Transaction &transaction)
{
    transactions.push_back(transaction);
    setTransactions(transactions);
}

void BackendDashboard::onTableSelected(QTreeWidgetItem *item, int column)
{
    // Desactivar temporalmente las señales de itemChanged para evitar interferencias
    ui->tablesTreeWidget->blockSignals(true);
    // Verificar si el item seleccionado es válido y no es el rootItem
    if (!item || item == rootItem) {
        ui->tablesTreeWidget->blockSignals(false); // Reactivar las señales antes de salir
        return;
    }

    qDebug() << "Table item selected: " << item->text(0);

    // Buscar la transacción correspondiente en `transactions`
    for (const auto &transaction : transactions) {
        if (transaction.getName() == item->text(0).toStdString()) {
            setCurrentTransaction(const_cast<Transaction &>(transaction));

            // Actualizar el nombre del label para que muestre el nombre de la tabla seleccionada
            ui->fieldLabel->setText(QString::fromStdString(transaction.getName()) + " Table");

            // Actualizar el nombre del label para que muestre el nombre de la tabla seleccionada
            ui->labelMethods->setText(QString::fromStdString(transaction.getName()) + " Methods");

            updateFieldsTable(transaction);
            break;
        }
    }
    // Reactivar las señales después de completar la actualización
    ui->tablesTreeWidget->blockSignals(false);
}

void BackendDashboard::updateFieldsTable(const Transaction &transaction)
{
    // Limpiar el contenido de la tabla de tareas
    ui->fieldsTableWidget->clearContents();
    ui->fieldsTableWidget->setRowCount(transaction.getFields().size());
    ui->fieldsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Agregar los datos de los campos a la tabla
    for (int row = 0; row < transaction.getFields().size(); ++row) {
        const Field &field = transaction.getFields()[row];
        ui->fieldsTableWidget
            ->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(field.getName())));
        ui->fieldsTableWidget
            ->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(field.getType())));

        // Crear elementos para Primary Key y Foreign Key con checkbox
        QTableWidgetItem *pkItem = new QTableWidgetItem();
        pkItem->setCheckState(field.isPrimaryKey() ? Qt::Checked : Qt::Unchecked);
        pkItem->setTextAlignment(Qt::AlignCenter);
        pkItem->setFlags(pkItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsUserCheckable);
        ui->fieldsTableWidget->setItem(row, 2, pkItem);

        QTableWidgetItem *fkItem = new QTableWidgetItem();
        fkItem->setCheckState(field.isForeignKey() ? Qt::Checked : Qt::Unchecked);
        fkItem->setTextAlignment(Qt::AlignCenter);
        fkItem->setFlags(fkItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsUserCheckable);

        ui->fieldsTableWidget->setItem(row, 3, fkItem);

        // Restricciones adicionales (UNIQUE, NULL, etc.)
        std::vector<std::string> constraints;
        QString constraintsLabel;
        if (field.getIsUnique()) {
            constraints.push_back("UNIQUE");
        }
        if (field.getIsNull()) {
            constraints.push_back("NULL");
        }

        for (size_t i = 0; i < constraints.size(); ++i) {
            constraintsLabel.append(constraints.at(i));
            if (constraints.size() - (i + 1) > 0)
                constraintsLabel.append("\n");
        }

        // Crear un elemento de la columna Const para mostrar restricciones adicionales
        ui->fieldsTableWidget->setItem(row, 4, new QTableWidgetItem(constraintsLabel));
    }

    // Ajustar el tamaño de las celdas para adaptarse al contenido
    ui->fieldsTableWidget->resizeColumnsToContents();
    ui->fieldsTableWidget->resizeRowsToContents();

    // Mantener un ancho mínimo para las columnas
    for (int column = 0; column < ui->fieldsTableWidget->columnCount(); ++column) {
        ui->fieldsTableWidget->setColumnWidth(column,
                                              30); // Definir el ancho mínimo para cada columna
    }
}

void BackendDashboard::setDatabaseLabel(const std::string &dbName)
{
    ui->tableLabel->setText(QString::fromStdString(dbName));
}

// Getters
std::vector<Transaction> &BackendDashboard::getTransactions()
{
    return transactions;
}

const std::vector<Transaction> &BackendDashboard::getTransactions() const
{
    return transactions;
}

// Setters
std::string toLowerCase(const std::string &str)
{
    std::string lowerCaseStr = str;
    std::transform(lowerCaseStr.begin(),
                   lowerCaseStr.end(),
                   lowerCaseStr.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return lowerCaseStr;
}

// Slots

void BackendDashboard::onTableNameChanged(QTreeWidgetItem *item, int column)
{
    if (!item || item == rootItem)
        return;

    QString newName = item->text(0);

    // Solo continuar si el nombre realmente ha cambiado
    if (newName == QString::fromStdString(currentTransaction.getName())) {
        return; // Si el nombre es el mismo, no hacer nada
    }

    // Actualizar la transacción correspondiente en la lista de transacciones
    for (auto &transaction : transactions) {
        if (transaction.getName() == currentTransaction.getName()) {
            transaction.setName(newName.toStdString());
            break;
        }
    }

    // Actualizar el nombre de la tabla actual
    currentTransaction.setName(newName.toStdString());

    // Actualizar los labels de la UI
    ui->fieldLabel->setText(newName + " Table");
    ui->labelMethods->setText(newName + " Methods");
}

void BackendDashboard::onFieldUpdated(const Field &updatedField)
{
    bool fieldUpdated = false;

    // Recorrer los campos de la transacción actual
    for (auto &existingField : currentTransaction.getFields()) {
        if (existingField.getName() == updatedField.getName()) {
            // Si encontramos un campo con el mismo nombre, actualizamos sus valores
            existingField = updatedField;
            fieldUpdated = true;
            break;
        }
    }

    // Actualizar la transacción en el vector de transacciones
    for (auto &transaction : transactions) {
        if (transaction.getName() == currentTransaction.getName()) {
            transaction.setFields(currentTransaction.getFields());
            break;
        }
    }

    updateFieldsTable(currentTransaction); // Actualizar la tabla visual
}

void BackendDashboard::on_deleteField_clicked()
{
    // Verificar si hay un campo seleccionado en la tabla de fields
    int selectedRow = ui->fieldsTableWidget->currentRow();
    // Verificar que haya una fila seleccionada
    if (selectedRow >= 0) {
        // Confirmar eliminación

        // Cuadro de diálogo de confirmación con estilos aplicados
        QMessageBox msgBox;
        msgBox.setStyleSheet(
            "QPushButton { background-color: #f0f0f0; color: black; padding: 5px 10px; }"
            "QMessageBox { background-color: white; }");

        msgBox.setWindowTitle("Delete Field");
        msgBox.setText("Are you sure you want to delete the selected field?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        int reply = msgBox.exec();

        if (reply == QMessageBox::Yes) {
            // Eliminar el campo del currentTransaction
            currentTransaction.getFields().erase(currentTransaction.getFields().begin()
                                                 + selectedRow);

            // Actualizar la tabla visual (QTableWidget)
            updateFieldsTable(currentTransaction);

            // Actualizar las transacciones en BackendGenerator
            for (auto &transaction : transactions) {
                if (transaction.getName() == currentTransaction.getName()) {
                    transaction.setFields(currentTransaction.getFields());
                    break;
                }
            }
        }
    } else {
        // Mostrar un mensaje de advertencia si no hay un campo seleccionado
        QMessageBox::warning(this, "No Selection", "Please select a field to delete.");
    }
}

void BackendDashboard::on_addField_clicked()
{
    if (!addFieldDialog) {
        addFieldDialog = new AddFieldDialog(this);

        connect(addFieldDialog, &AddFieldDialog::fieldSaved, this, &BackendDashboard::onFieldSaved);
    }

    // Asegúrate de que `currentTransaction` esté asignado
    if (currentTransaction.getName().empty()) {
        QMessageBox::warning(this, "Error", "No transaction is currently selected.");
        return;
    }

    // Supongamos que tienes una lista de transacciones disponibles
    std::vector<QString> tableNames;
    for (const auto &transaction :
         transactions) { // Suponiendo que 'transactions' es tu vector de transacciones
        tableNames.push_back(QString::fromStdString(transaction.getName()));
    }

    QString currentTableName = QString::fromStdString(currentTransaction.getName());
    addFieldDialog->setAvailableTables(tableNames, currentTableName);

    // Asignar el currentTransaction al AddFieldDialog
    addFieldDialog->setTransaction(currentTransaction);

    addFieldDialog->exec();
}

void BackendDashboard::on_editField_clicked()
{
    int selectedRow = ui->fieldsTableWidget->currentRow(); // Obtener la fila seleccionada

    // Verificar que haya un campo seleccionado
    if (selectedRow < 0 || selectedRow >= currentTransaction.getFields().size()) {
        QMessageBox::warning(this, "No Selection", "Please select a field to edit.");
        return;
    }

    // Obtener referencia al campo seleccionado
    Field &selectedField = currentTransaction.getFields()[selectedRow];

    // Inicializar el diálogo si no está inicializado
    if (!editFieldDialog) {
        editFieldDialog = new EditFieldDialog(this);
        connect(editFieldDialog,
                &EditFieldDialog::fieldSaved,
                this,
                &BackendDashboard::onFieldSaved);
    }

    // Establecer los datos actuales del campo seleccionado en el diálogo de edición
    editFieldDialog->setField(selectedField);

    // Ejecutar el diálogo
    if (editFieldDialog->exec() == QDialog::Accepted) {
        Field updatedField = editFieldDialog->getField();

        // Actualizar el campo editado en la posición correspondiente
        currentTransaction.getFields()[selectedRow] = updatedField;

        // Actualizar las transacciones en la lista general
        for (auto &transaction : transactions) {
            if (transaction.getName() == currentTransaction.getName()) {
                transaction.setFields(currentTransaction.getFields());
                break;
            }
        }
        // Actualizar la tabla visual (QTableWidget)
        updateFieldsTable(currentTransaction);
    }
}

void BackendDashboard::on_deleteTable_clicked()
{
    // Obtener el elemento seleccionado en el árbol de tablas
    QTreeWidgetItem *selectedItem = ui->tablesTreeWidget->currentItem();

    // Verificar que haya un elemento seleccionado y que no sea el rootItem
    if (!selectedItem || selectedItem == rootItem) {
        return; // Si no hay un elemento seleccionado o es el nodo raíz, no hacer nada
    }

    QString tableName = selectedItem->text(0); // Nombre de la tabla seleccionada

    // Mostrar un cuadro de diálogo para confirmar la eliminación
    // Crear y mostrar un cuadro de diálogo para confirmar la eliminación con estilos aplicados
    QMessageBox msgBox;
    msgBox.setStyleSheet(
        "QPushButton { background-color: #f0f0f0; color: black; padding: 5px 10px; }"
        "QMessageBox { background-color: white; }");

    msgBox.setWindowTitle("Delete Table");
    msgBox.setText("Are you sure you want to delete the table '" + tableName + "'?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    int reply = msgBox.exec();

    if (reply == QMessageBox::Yes) {
        // Eliminar el elemento del árbol visual
        delete selectedItem;

        // Eliminar la transacción correspondiente en la lista de transacciones
        auto it = std::remove_if(transactions.begin(),
                                 transactions.end(),
                                 [&tableName](const Transaction &transaction) {
                                     return transaction.getName() == tableName.toStdString();
                                 });
        transactions.erase(it, transactions.end());

        // Limpiar la tabla de tareas asociada
        ui->fieldsTableWidget->clearContents();
        ui->fieldsTableWidget->setRowCount(0);

        // Actualizar las etiquetas y la interfaz gráfica
        ui->fieldLabel->setText("No Table Selected");
        ui->labelMethods->setText("No Methods Available");
    }
}

void BackendDashboard::on_createTable_clicked()
{
    if (!createTableDialog) {
        createTableDialog = new CreateTableDialog(this);

        connect(createTableDialog,
                &CreateTableDialog::transactionSaved,
                this,
                &BackendDashboard::onTransactionSaved);
    }
    createTableDialog->exec();
}

void BackendDashboard::on_editTable_clicked()
{
    QTreeWidgetItem *selectedItem = ui->tablesTreeWidget->currentItem();
    if (!selectedItem || selectedItem == rootItem)
        return; // Si no hay nada seleccionado o es el nodo raíz, no hacer nada

    QString currentName = selectedItem->text(0);

    // Crear un diálogo personalizado
    QDialog dialog(this);
    dialog.setWindowTitle("Edit Table Name");

    // Crear un layout vertical para los widgets
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    // Crear un label y un line edit para el nuevo nombre
    QLabel *label = new QLabel("New table name:", &dialog);
    QLineEdit *lineEdit = new QLineEdit(currentName, &dialog);

    // Crear botones OK y Cancel
    QPushButton *okButton = new QPushButton("Save", &dialog);
    QPushButton *cancelButton = new QPushButton("Cancel", &dialog);

    // Aplicar estilos a los botones
    okButton->setStyleSheet("background-color: #0F66DE; padding: 5px 10px;");
    cancelButton->setStyleSheet("background-color: #F44336; padding: 5px 10px;");

    // Crear un layout horizontal para los botones
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    // Añadir los widgets al layout principal
    layout->addWidget(label);
    layout->addWidget(lineEdit);
    layout->addLayout(buttonLayout);

    // Conectar los botones a las funciones de aceptación y rechazo
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    // Mostrar el diálogo
    if (dialog.exec() == QDialog::Accepted) {
        QString newName = lineEdit->text();

        if (!newName.isEmpty() && newName != currentName) {
            // Actualizar el nombre en la interfaz gráfica
            selectedItem->setText(0, newName);

            // Actualizar el nombre en la lista de transacciones
            for (auto &transaction : transactions) {
                if (transaction.getName() == currentName.toStdString()) {
                    transaction.setName(newName.toStdString());
                    transaction.setNameConst(newName.toLower().toStdString());
                    // Actualizar los labels de la UI
                    ui->fieldLabel->setText(newName + " Table");
                    ui->labelMethods->setText(newName + " Methods");

                    break;
                }
            }
        }
    }
}
