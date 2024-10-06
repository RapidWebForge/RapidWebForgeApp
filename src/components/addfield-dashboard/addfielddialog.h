#ifndef ADDFIELDDIALOG_H
#define ADDFIELDDIALOG_H

#include <QDialog>

namespace Ui {
class AddFieldDialog;
}

class AddFieldDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddFieldDialog(QWidget *parent = nullptr);
    ~AddFieldDialog();

private:
    Ui::AddFieldDialog *ui;
};

#endif // ADDFIELDDIALOG_H
