#ifndef ADDFIELDDIALOG_H
#define ADDFIELDDIALOG_H

#include <QDialog>
#include "../../models/field/field.h"
#include "../../models/transaction/transaction.h" // Incluir el modelo de transacción

namespace Ui {
class AddFieldDialog;
}

class AddFieldDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddFieldDialog(QWidget *parent = nullptr);
    ~AddFieldDialog();

    void setTransaction(Transaction &transaction);

signals:
    void fieldSaved(const Field &field);

private slots:
    void on_addButton_clicked();

private:
    Ui::AddFieldDialog *ui;
    Field field;
    Transaction *currentTransaction; // Referencia a la transacción actual
};

#endif // ADDFIELDDIALOG_H
