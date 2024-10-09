#ifndef BACKENDDASHBOARD_H
#define BACKENDDASHBOARD_H

#include <QCheckBox>
#include <QDialog>
#include <QTreeWidgetItem>
#include "../../models/transaction/transaction.h"
#include "../addfield-dashboard/addfielddialog.h"
#include "../create-table-dashboard/createtabledialog.h"

namespace Ui {
class BackendDashboard;
}

class BackendDashboard : public QDialog
{
    Q_OBJECT

public:
    explicit BackendDashboard(QWidget *parent = nullptr);
    ~BackendDashboard();
    // Getters
    const std::vector<Transaction> &getTransactions() const;
    std::vector<Transaction> &getTransactions();
    // Setters
    void setTransactions(const std::vector<Transaction> &newTransactions);
    void setCurrentTransaction(Transaction &transaction);
    // Método para configurar el nombre de la base de datos
    void setDatabaseLabel(const std::string &dbName);

public slots:
    void onFieldSaved(const Field &field);
    void onTransactionSaved(const Transaction &transaction);
    // Nueva función para manejar la selección de una tabla
    void onTableSelected(QTreeWidgetItem *item, int column);

private slots:
    void showCreateTableDialog();
    void showAddFieldDialog();

private:
    Ui::BackendDashboard *ui;
    void applyStylesBack();
    void setupTasksTable();
    void setupTasksMethodsList();
    void updateTasksTable(
        const Transaction &transaction); // Nueva función para actualizar la tabla de tareas

    CreateTableDialog *createTableDialog;
    AddFieldDialog *addFieldDialog;
    QTreeWidgetItem *rootItem;
    std::vector<Transaction> transactions;
    Transaction currentTransaction;
};

#endif // BACKENDDASHBOARD_H
