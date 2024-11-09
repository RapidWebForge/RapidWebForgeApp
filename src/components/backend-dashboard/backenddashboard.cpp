#include "backenddashboard.h"
#include <QFile>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include "ui_backenddashboard.h"
#include <algorithm>
#include <cctype>

BackendDashboard::BackendDashboard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::BackendDashboard)
    , createTableDialog(nullptr) // Inicialización del puntero a nullptr
    , addFieldDialog(nullptr)
    , editFieldDialog(nullptr) // Inicializar editFieldDialog a nullptr
    , rootItem(nullptr)
{
    ui->setupUi(this);

    // Create and setting root
    rootItem = new QTreeWidgetItem(ui->databaseTreeWidget);
    rootItem->setText(0, "Database tables");
    rootItem->setIcon(0, QIcon(":/icons/database.png"));

    ui->databaseTreeWidget->expandAll();

    // Conectar el evento de selección del árbol de tablas a la función onTableSelected
    connect(ui->databaseTreeWidget,
            &QTreeWidget::itemClicked,
            this,
            &BackendDashboard::onTableSelected);

    connect(ui->addButton, &QPushButton::clicked, this, &BackendDashboard::showAddFieldDialog);

    connect(ui->deleteDB, &QPushButton::clicked, this, &BackendDashboard::on_deleteButton_clicked);

    connect(ui->deleteButton,
            &QPushButton::clicked,
            this,
            &BackendDashboard::on_deleteFieldButton_clicked);

    //connect(ui->databaseTreeWidget,&QTreeWidget::itemChanged,this,&BackendDashboard::onTableNameChanged);

    //connect(ui->tasksTableWidget,&QTableWidget::cellChanged,this,&BackendDashboard::onCellChanged);

    setupTasksMethodsList();
    setupTasksTable();
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

    ui->databaseLabel->setStyleSheet("font-size: 16px; color: #27292A; padding-top: 0px; "
                                     "padding-left: 10px; padding-bottom: 10px;");

    ui->labelTable->setStyleSheet("font-size: 16px; color: #27292A; padding-top: 0px; "
                                  "padding-left: 10px; padding-bottom: 10px;");

    ui->labelMethods->setStyleSheet("font-size: 16px; color: #27292A; padding-top: 0px; "
                                    "padding-left: 10px; padding-bottom: 10px;");

    // Puedes ajustar los iconos y tamaño de los botones
    ui->editButton->setIcon(QIcon(":/icons/edit.png"));
    ui->editButton->setIconSize(QSize(16, 16));
    ui->editButton->setToolTip("Edit table");

    ui->addButton->setIcon(QIcon(":/icons/add.png"));
    ui->addButton->setIconSize(QSize(16, 16));
    ui->addButton->setToolTip("Add table");

    ui->deleteButton->setIcon(QIcon(":/icons/delete.png"));
    ui->deleteButton->setIconSize(QSize(16, 16));
    ui->deleteButton->setToolTip("Delete table");

    ui->createTableButton->setIcon(QIcon(":/icons/adddb.png"));
    ui->createTableButton->setIconSize(QSize(16, 16));
    ui->createTableButton->setToolTip("Delete table");

    ui->deleteDB->setIcon(QIcon(":/icons/delete.png"));
    ui->deleteDB->setIconSize(QSize(16, 16));
    ui->deleteDB->setToolTip("Delete table");

    ui->editDB->setIcon(QIcon(":/icons/edit.png"));
    ui->editDB->setIconSize(QSize(16, 16));
    ui->editDB->setToolTip("Delete table");
}

void BackendDashboard::setupTasksTable()
{
    // Configurar columnas y filas
    ui->tasksTableWidget->setColumnCount(5);
    QStringList headers;
    headers << "Field name" << "Type" << "PK" << "FK" << "Const";
    ui->tasksTableWidget->setHorizontalHeaderLabels(headers);

    // Configurar la propiedad de ajuste de texto (WordWrap)
    ui->tasksTableWidget->setWordWrap(true);

    // Configurar el tamaño de las celdas para ajustarse al contenido
    ui->tasksTableWidget->resizeColumnsToContents();
    ui->tasksTableWidget->resizeRowsToContents();

    // Ajustar el tamaño de las celdas para adaptarse al contenido automáticamente
    ui->tasksTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tasksTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Ajustes de estilo y visualización
    ui->tasksTableWidget->horizontalHeader()->setStretchLastSection(
        true); // Última columna ajustada al ancho restante
    ui->tasksTableWidget->verticalHeader()->setVisible(false); // Oculta el encabezado vertical
    ui->tasksTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows); // Selección por filas
    ui->tasksTableWidget->setEditTriggers(QAbstractItemView::DoubleClicked
                                          | QAbstractItemView::SelectedClicked);

    // Ajustes de estilo
    ui->tasksTableWidget->setStyleSheet("QTableWidget {"
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
    for (int row = 0; row < ui->tasksTableWidget->rowCount(); ++row) {
        QTableWidgetItem *pkItem = new QTableWidgetItem();
        pkItem->setCheckState(Qt::Unchecked);
        pkItem->setTextAlignment(Qt::AlignCenter);
        ui->tasksTableWidget->setItem(row, 2, pkItem);

        QTableWidgetItem *fkItem = new QTableWidgetItem();
        fkItem->setCheckState(Qt::Unchecked);
        fkItem->setTextAlignment(Qt::AlignCenter);
        ui->tasksTableWidget->setItem(row, 3, fkItem);
    }

    // Ajustes de visualización
    ui->tasksTableWidget->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);                                 // Extiende las columnas
    ui->tasksTableWidget->verticalHeader()->setVisible(false); // Oculta el encabezado vertical
    ui->tasksTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows); // Selección por filas
}

void BackendDashboard::setupTasksMethodsList()
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

void BackendDashboard::showAddFieldDialog()
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

// Setters
void BackendDashboard::setTransactions(const std::vector<Transaction> &newTransactions)
{
    transactions = newTransactions;

    // Clear rootItem
    rootItem->takeChildren();

    // Add transactions like children
    for (const auto &transaction : transactions) {
        QTreeWidgetItem *item = new QTreeWidgetItem(rootItem);
        item->setText(0, QString::fromStdString(transaction.getName()));
    }

    // Expandir todo el árbol para mostrar todas las tablas
    ui->databaseTreeWidget->expandAll();

    // If transactions are available, set the first one as the current transaction
    if (!transactions.empty()) {
        // Load the first transaction automatically
        setCurrentTransaction(transactions[0]);
        updateTasksTable(transactions[0]);

        // Update UI labels for the first transaction
        ui->labelTable->setText(QString::fromStdString(transactions[0].getName()) + " Table");
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
    updateTasksTable(currentTransaction);
}

void BackendDashboard::onTransactionSaved(const Transaction &transaction)
{
    transactions.push_back(transaction);
    setTransactions(transactions);
}

// Definición de la función onTableSelected
void BackendDashboard::onTableSelected(QTreeWidgetItem *item, int column)
{
    // Desactivar temporalmente las señales de itemChanged para evitar interferencias
    ui->databaseTreeWidget->blockSignals(true);
    // Verificar si el item seleccionado es válido y no es el rootItem
    if (!item || item == rootItem) {
        ui->databaseTreeWidget->blockSignals(false); // Reactivar las señales antes de salir
        return;
    }

    qDebug() << "Table item selected: " << item->text(0); // Añade esto para verificar la ejecución

    // Buscar la transacción correspondiente en `transactions`
    for (const auto &transaction : transactions) {
        if (transaction.getName() == item->text(0).toStdString()) {
            setCurrentTransaction(const_cast<Transaction &>(transaction));

            // Actualizar el nombre del label para que muestre el nombre de la tabla seleccionada
            ui->labelTable->setText(QString::fromStdString(transaction.getName()) + " Table");

            // Actualizar el nombre del label para que muestre el nombre de la tabla seleccionada
            ui->labelMethods->setText(QString::fromStdString(transaction.getName()) + " Methods");

            updateTasksTable(transaction);
            break;
        }
    }
    // Reactivar las señales después de completar la actualización
    ui->databaseTreeWidget->blockSignals(false);
}

// Definición de la función updateTasksTable
void BackendDashboard::updateTasksTable(const Transaction &transaction)
{
    // Limpiar el contenido de la tabla de tareas
    ui->tasksTableWidget->clearContents();
    ui->tasksTableWidget->setRowCount(transaction.getFields().size());
    ui->tasksTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Agregar los datos de los campos a la tabla
    for (int row = 0; row < transaction.getFields().size(); ++row) {
        const Field &field = transaction.getFields()[row];
        ui->tasksTableWidget->setItem(row,
                                      0,
                                      new QTableWidgetItem(QString::fromStdString(field.getName())));
        ui->tasksTableWidget->setItem(row,
                                      1,
                                      new QTableWidgetItem(QString::fromStdString(field.getType())));

        // Crear elementos para Primary Key y Foreign Key con checkbox
        QTableWidgetItem *pkItem = new QTableWidgetItem();
        pkItem->setCheckState(field.isPrimaryKey() ? Qt::Checked : Qt::Unchecked);
        pkItem->setTextAlignment(Qt::AlignCenter);
        pkItem->setFlags(pkItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsUserCheckable);
        ui->tasksTableWidget->setItem(row, 2, pkItem);

        QTableWidgetItem *fkItem = new QTableWidgetItem();
        fkItem->setCheckState(field.isForeignKey() ? Qt::Checked : Qt::Unchecked);
        fkItem->setTextAlignment(Qt::AlignCenter);
        fkItem->setFlags(fkItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsUserCheckable);

        ui->tasksTableWidget->setItem(row, 3, fkItem);

        // Restricciones adicionales (UNIQUE, NULL, etc.)
        QString constraints;
        if (field.getIsUnique()) {
            constraints.append("UNIQUE ");
        }
        if (field.getIsNull()) {
            constraints.append("NULL ");
        }
        // Crear un elemento de la columna Const para mostrar restricciones adicionales como UNIQUE o CHECK
        ui->tasksTableWidget->setItem(row,
                                      4,
                                      new QTableWidgetItem(QString::fromStdString(
                                          field.getIsUnique() ? "UNIQUE" : "")));
    }

    // Ajustar el tamaño de las celdas para adaptarse al contenido
    ui->tasksTableWidget->resizeColumnsToContents();
    ui->tasksTableWidget->resizeRowsToContents();

    // Mantener un ancho mínimo para las columnas
    for (int column = 0; column < ui->tasksTableWidget->columnCount(); ++column) {
        ui->tasksTableWidget->setColumnWidth(column,
                                             30); // Definir el ancho mínimo para cada columna
    }
}

// Función para actualizar el nombre de la base de datos en el QLabel
void BackendDashboard::setDatabaseLabel(const std::string &dbName)
{
    ui->databaseLabel->setText(QString::fromStdString(dbName));
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

void BackendDashboard::on_deleteButton_clicked()
{
    // Obtener el elemento seleccionado en el árbol de tablas
    QTreeWidgetItem *selectedItem = ui->databaseTreeWidget->currentItem();

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
        ui->tasksTableWidget->clearContents();
        ui->tasksTableWidget->setRowCount(0);

        // Actualizar las etiquetas y la interfaz gráfica
        ui->labelTable->setText("No Table Selected");
        ui->labelMethods->setText("No Methods Available");
    }
}

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
    ui->labelTable->setText(newName + " Table");
    ui->labelMethods->setText(newName + " Methods");
}

void BackendDashboard::on_deleteFieldButton_clicked()
{
    // Verificar si hay un campo seleccionado en la tabla de fields
    int selectedRow = ui->tasksTableWidget->currentRow(); // Obtener la fila seleccionada
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
            updateTasksTable(currentTransaction);

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

    updateTasksTable(currentTransaction); // Actualizar la tabla visual
}

void BackendDashboard::on_editDB_clicked()
{
    QTreeWidgetItem *selectedItem = ui->databaseTreeWidget->currentItem();
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
                    ui->labelTable->setText(newName + " Table");
                    ui->labelMethods->setText(newName + " Methods");

                    break;
                }
            }
        }
    }
}

void BackendDashboard::on_editButton_clicked()
{
    int selectedRow = ui->tasksTableWidget->currentRow(); // Obtener la fila seleccionada

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
        updateTasksTable(currentTransaction);
    }
}

void BackendDashboard::on_createTableButton_clicked()
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
