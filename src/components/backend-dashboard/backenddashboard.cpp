#include "backenddashboard.h"
#include "ui_backenddashboard.h"

BackendDashboard::BackendDashboard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::BackendDashboard)
    , createTableDialog(nullptr) // Inicialización del puntero a nullptr
    , addFieldDialog(nullptr)
    , rootItem(nullptr)
{
    ui->setupUi(this);

    // Create and setting root
    rootItem = new QTreeWidgetItem(ui->databaseTreeWidget);
    rootItem->setText(0, "Database tables");
    rootItem->setIcon(0, QIcon(":/icons/database.png"));

    ui->databaseTreeWidget->expandAll();

    connect(ui->pushButton, &QPushButton::clicked, this, &BackendDashboard::showCreateTableDialog);
    connect(ui->addButton, &QPushButton::clicked, this, &BackendDashboard::showAddFieldDialog);

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
    this->setStyleSheet("QTreeWidget {"
                        "   border: 1px solid #eaeaea;"
                        "   border-radius: 8px;"
                        "   background-color: white;"
                        "   font-size: 14px;"
                        "   margin: 10px;"
                        "   padding: 5px;"
                        "}"
                        "QTreeWidget::item {"
                        "   border-radius: 5px;"
                        "   margin: 2px;"
                        "   padding: 5px;"
                        "}"
                        "QTreeWidget::item:selected {"
                        "   background-color: #0F66DE;"
                        "   color: white;"
                        "}"
                        "QGroupBox {"
                        "   border: 1px solid #dcdcdc;"
                        "   border-radius: 8px;"
                        "   margin-top: 15px;"
                        "   padding: 10px;"
                        "   font-size: 16px;"
                        "   color: #333;"
                        "}"
                        "QTreeWidget, QTableWidget, QListWidget {"
                        "   border: 1px solid #eaeaea;"
                        "   border-radius: 5px;"
                        "   font-size: 14px;"
                        "   background-color: #fafafa;"
                        "}"
                        "QPushButton {"
                        "   background-color: white;"
                        "   color: white;"
                        "   border: none;"
                        "   border-radius: 12px;"
                        "   padding: 5px 5px;"
                        "   font-size: 12px;"
                        "}"
                        "QPushButton:hover {"
                        "   background-color: #eaeaea;"
                        "   color: white;"
                        "}"
                        "QPushButton:pressed {"
                        "   background-color: #eaeaea;"
                        "}"
                        "QLabel#titleLabel {"
                        "   font-size: 36px;"
                        "   font-weight: bold;"
                        "   color: #333;"
                        "}"
                        "QGroupBox {"
                        "   border: 1px solid #dcdcdc;"
                        "   border-radius: 8px;"
                        "   padding: 10px;"
                        "   font-size: 16px;"
                        "   color: #333;"
                        "}");

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

    ui->pushButton->setIcon(QIcon(":/icons/adddb.png"));
    ui->pushButton->setIconSize(QSize(16, 16));
    ui->pushButton->setToolTip("Delete table");
}

void BackendDashboard::setupTasksTable()
{
    // Configurar columnas y filas
    ui->tasksTableWidget->setColumnCount(5);
    QStringList headers;
    headers << "Field name" << "Type" << "PK" << "FK" << "Const";
    ui->tasksTableWidget->setHorizontalHeaderLabels(headers);

    ui->tasksTableWidget->setRowCount(4); // Número de filas

    // Agregar datos a la tabla
    ui->tasksTableWidget->setItem(0, 0, new QTableWidgetItem("id"));
    ui->tasksTableWidget->setItem(0, 1, new QTableWidgetItem("int"));
    ui->tasksTableWidget->setItem(0, 2, new QTableWidgetItem(""));
    ui->tasksTableWidget->setItem(0, 3, new QTableWidgetItem(""));
    ui->tasksTableWidget->setItem(0, 4, new QTableWidgetItem("NOT NULL"));

    ui->tasksTableWidget->setItem(1, 0, new QTableWidgetItem("title"));
    ui->tasksTableWidget->setItem(1, 1, new QTableWidgetItem("chart(55)"));
    ui->tasksTableWidget->setItem(1, 2, new QTableWidgetItem(""));
    ui->tasksTableWidget->setItem(1, 3, new QTableWidgetItem(""));
    ui->tasksTableWidget->setItem(1, 4, new QTableWidgetItem("UNIQUE"));

    ui->tasksTableWidget->setItem(2, 0, new QTableWidgetItem("description"));
    ui->tasksTableWidget->setItem(2, 1, new QTableWidgetItem("chart(55)"));
    ui->tasksTableWidget->setItem(2, 2, new QTableWidgetItem(""));
    ui->tasksTableWidget->setItem(2, 3, new QTableWidgetItem(""));
    ui->tasksTableWidget->setItem(2, 4, new QTableWidgetItem("CHECK"));

    ui->tasksTableWidget->setItem(3, 0, new QTableWidgetItem("creation date"));
    ui->tasksTableWidget->setItem(3, 1, new QTableWidgetItem("chart(55)"));
    ui->tasksTableWidget->setItem(3, 2, new QTableWidgetItem(""));
    ui->tasksTableWidget->setItem(3, 3, new QTableWidgetItem(""));
    ui->tasksTableWidget->setItem(3, 4, new QTableWidgetItem("DEFAULT"));

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

    // Ajustes de estilo
    ui->tasksTableWidget->setStyleSheet("QTableWidget {"
                                        "   background-color: #ffffff;"
                                        "   border: 1px solid #dcdcdc;"
                                        "   border-radius: 8px;"
                                        "   font-size: 14px;"
                                        "   color: #333;"
                                        "}"
                                        "QTableWidget::item {"
                                        "   padding: 10px;"
                                        "}"
                                        "QTableWidget::item:selected {"
                                        "   background-color: #0F66DE;"
                                        "   color: white;"
                                        "}");
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

void BackendDashboard::showCreateTableDialog()
{
    if (!createTableDialog) {
        createTableDialog = new CreateTableDialog(this);
    }
    createTableDialog->exec(); // Muestra el diálogo de forma modal
}

void BackendDashboard::showAddFieldDialog()
{
    if (!addFieldDialog) {
        addFieldDialog = new AddFieldDialog(this);
    }
    addFieldDialog->exec(); // Muestra el diálogo de forma modal
}

void BackendDashboard::setTransactions(const std::vector<Transaction> &newTransactions)
{
    transactions = newTransactions;

    // Clear rootItem
    rootItem->takeChildren();

    // Add transactions like children
    for (const auto &transaction : transactions) {
        QTreeWidgetItem *item = new QTreeWidgetItem(rootItem);
        item->setText(0, QString::fromStdString(transaction.getName()));
        item->setIcon(0, QIcon(":/icons/table.png"));
        // item->setIcon(0, QIcon(":/icons/delete.png"));
        // TODO: Replace for a custom widget to allow multiple icons
    }

    // Expandir todo el árbol para mostrar todas las tablas
    ui->databaseTreeWidget->expandAll();
}
std::vector<Transaction> &BackendDashboard::getTransactions()
{
    return transactions;
}

const std::vector<Transaction> &BackendDashboard::getTransactions() const
{
    return transactions;
}
