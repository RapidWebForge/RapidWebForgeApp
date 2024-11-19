#ifndef ADDFIELDDIALOG_H
#define ADDFIELDDIALOG_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QString>
#include "../../models/field/field.h"
#include "../../models/transaction/transaction.h" // Incluir el modelo de transacción
#include <vector>

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
    void setAvailableTables(const std::vector<QString> &tables,
                            const QString &currentTableName); // Método para llenar el combo box

signals:
    void fieldSaved(const Field &field);

private slots:
    void on_addButton_clicked();
    void on_foreignKeyCheckBox_stateChanged(
        int state); // Maneja el cambio de estado del checkbox de clave foránea

private:
    Ui::AddFieldDialog *ui;
    Field field;
    Transaction *currentTransaction; // Referencia a la transacción actual

    void applyStyles();
};

#endif // ADDFIELDDIALOG_H
