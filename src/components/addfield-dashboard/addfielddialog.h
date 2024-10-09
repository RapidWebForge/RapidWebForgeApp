#ifndef ADDFIELDDIALOG_H
#define ADDFIELDDIALOG_H

#include <QDialog>
#include "../../models/field/field.h"

namespace Ui {
class AddFieldDialog;
}

class AddFieldDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddFieldDialog(QWidget *parent = nullptr);
    ~AddFieldDialog();

signals:
    void fieldSaved(const Field &field);

private slots:
    void on_addButton_clicked();

private:
    Ui::AddFieldDialog *ui;
    Field field;
};

#endif // ADDFIELDDIALOG_H
