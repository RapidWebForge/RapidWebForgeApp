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
    // Setters
    void setTransactions(std::vector<Transaction> *transactionsRef);
    void setCurrentTransaction(Transaction *transaction);
    void setCurrentField(Field &field);
    void setDatabaseLabel(const std::string &dbName);

public slots:
    void onFieldSaved();
    void onTransactionSaved(const Transaction &transaction);
    void onTableSelected(QTreeWidgetItem *item, int column);
    void onFieldSelected(int row, int column);
    void onTableNameChanged(QTreeWidgetItem *item, int column);

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
    void updateFieldsTable();
    void updateTablesTree();

    CreateTableDialog *createTableDialog;
    AddFieldDialog *addFieldDialog;
    QTreeWidgetItem *rootItem;
    std::vector<Transaction> *transactions;
    Transaction *currentTransaction = nullptr;
    Field *currentField = nullptr;
    EditFieldDialog *editFieldDialog;
};

#endif // BACKENDDASHBOARD_H
