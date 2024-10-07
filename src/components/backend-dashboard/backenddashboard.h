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
    std::vector<Transaction> transactions;
    void setTransactions(const std::vector<Transaction> &newTransactions);
    const std::vector<Transaction> &getTransactions() const;
    std::vector<Transaction> &getTransactions();

private slots:
    void showCreateTableDialog();
    void showAddFieldDialog();

private:
    Ui::BackendDashboard *ui;
    void applyStylesBack();
    void setupTasksTable();
    void setupTasksMethodsList();
    CreateTableDialog *createTableDialog;
    AddFieldDialog *addFieldDialog;
    QTreeWidgetItem *rootItem;
};

#endif // BACKENDDASHBOARD_H
