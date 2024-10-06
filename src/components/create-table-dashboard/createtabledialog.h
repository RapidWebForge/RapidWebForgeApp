#ifndef CREATETABLEDIALOG_H
#define CREATETABLEDIALOG_H

#include <QDialog>
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
private slots:
    void showAddFieldDialog();

private:
    Ui::CreateTableDialog *ui;
    AddFieldDialog *addFieldDialog;
};

#endif // CREATETABLEDIALOG_H
