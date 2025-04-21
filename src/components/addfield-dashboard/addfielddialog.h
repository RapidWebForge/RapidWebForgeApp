#ifndef ADDFIELDDIALOG_H
#define ADDFIELDDIALOG_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QString>
#include "../../core/logging/actionloggerjson.h" // Para manejar logs en formato .json
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

    void setTransaction(Transaction *transaction);
    // Método para llenar el combo box
    void setAvailableTables(const std::vector<QString> &tables, const QString &currentTableName);

signals:
    void fieldSaved();

private slots:
    void on_cancelButton_clicked();
    void on_addButton_clicked();
    // Maneja el cambio de estado del checkbox de clave foránea
    void on_foreignKeyCheckBox_stateChanged(int state);

private:
    Ui::AddFieldDialog *ui;
    Field field;
    Transaction *currentTransaction; // Referencia a la transacción actual
    ActionLoggerJson loggerJson;     // Logs en formato .json

    void applyStyles();
    void clearContent();
};

#endif // ADDFIELDDIALOG_H
