#ifndef CREATETABLEDIALOG_H
#define CREATETABLEDIALOG_H

#include <QDialog>
#include "../../models/transaction/transaction.h"
#include "../addfield-dashboard/addfielddialog.h"

namespace Ui {
class CreateTableDialog;
}

class CreateTableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateTableDialog(QWidget *parent = nullptr);
    ~CreateTableDialog();

signals:
    void transactionSaved(const Transaction &transaction);

public slots:
    void onFieldSaved(const Field &field);

private slots:
    void showAddFieldDialog();

    void on_createButton_clicked();

private:
    Ui::CreateTableDialog *ui;
    AddFieldDialog *addFieldDialog;
    Transaction transaction;
};

#endif // CREATETABLEDIALOG_H
