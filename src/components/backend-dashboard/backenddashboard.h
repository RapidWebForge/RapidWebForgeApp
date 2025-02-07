#ifndef BACKENDDASHBOARD_H
#define BACKENDDASHBOARD_H

#include <QCheckBox>
#include <QTreeWidgetItem>
#include <QWidget>
#include "../../models/transaction/transaction.h"
#include "../addfield-dashboard/addfielddialog.h"
#include "../create-table-dashboard/createtabledialog.h"
#include "../editfield-dashboard/editfielddialog.h"
#include <vector>

namespace Ui {
class BackendDashboard;
}

class BackendDashboard : public QWidget
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
    void setDatabaseLabel(const std::string &dbName);

public slots:
    void onFieldSaved(const Field &field);
    void onTransactionSaved(const Transaction &transaction);
    void onTableSelected(QTreeWidgetItem *item, int column);
    void onTableNameChanged(QTreeWidgetItem *item, int column);
    void onFieldUpdated(const Field &updatedField);

private slots:
    void on_deleteField_clicked();
    void on_addField_clicked();
    void on_editField_clicked();

    void on_deleteTable_clicked();
    void on_createTable_clicked();
    void on_editTable_clicked();

private:
    Ui::BackendDashboard *ui;
    void applyStylesBack();
    void setupFieldsTable();
    void setupMethodsList();
    void updateFieldsTable(const Transaction &transaction);

    CreateTableDialog *createTableDialog;
    AddFieldDialog *addFieldDialog;
    QTreeWidgetItem *rootItem;
    std::vector<Transaction> transactions;
    Transaction currentTransaction;
    EditFieldDialog *editFieldDialog;

signals:
    void transactionNameChanged(); // Señal emitida cuando se cambie el nombre de una transacción
    void fieldEdited(const Field &field); // Señal que se emite cuando un campo es editado
};

#endif // BACKENDDASHBOARD_H
