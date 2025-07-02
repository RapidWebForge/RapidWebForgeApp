#ifndef CREATETABLEDIALOG_H
#define CREATETABLEDIALOG_H

#include <QDialog>
#include "../../core/logging/actionloggerjson.h"
#include "../../models/transaction/transaction.h"

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

private slots:
    void on_createButton_clicked();

private:
    Ui::CreateTableDialog *ui;
    Transaction transaction;
    ActionLoggerJson loggerJson; // Logs en formato .json

    void applyStyles();
};

#endif // CREATETABLEDIALOG_H
