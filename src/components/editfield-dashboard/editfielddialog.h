#ifndef EDITFIELDDIALOG_H
#define EDITFIELDDIALOG_H

#include <QDialog>
#include "../../models/field/field.h"

namespace Ui {
class EditFieldDialog;
}

class EditFieldDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditFieldDialog(QWidget *parent = nullptr);
    ~EditFieldDialog();

    void setField(const Field &field); // Para cargar los datos del field en el diálogo
    Field getField() const;            // Para obtener el field actualizado del diálogo

signals:
    // Señal que se emite cuando se guarda un campo
    void fieldSaved(const Field &field); // Declaración de la señal

private slots:
    void on_acceptButton_clicked();

private:
    Ui::EditFieldDialog *ui;
    Field currentField; // El field que se está editando
};

#endif // EDITFIELDDIALOG_H
