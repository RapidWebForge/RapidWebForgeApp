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

    // Dentro de EditFieldDialog
    void setField(Field *field);

signals:
    // Señal que se emite cuando se guarda un field
    void fieldSaved();

private slots:
    void on_acceptButton_clicked();

private:
    Ui::EditFieldDialog *ui;
    Field *currentField = nullptr;

    void setUpWidget();
};

#endif // EDITFIELDDIALOG_H
