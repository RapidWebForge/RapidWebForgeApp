#ifndef BACKENDDASHBOARD_H
#define BACKENDDASHBOARD_H

#include <QCheckBox>
#include <QDialog>
#include "../addfield-dashboard/addfielddialog.h"
#include "../create-table-dashboard/createtabledialog.h"

namespace Ui {
class BackendDashboard;
}

class BackendDashboard : public QDialog
{
    Q_OBJECT

public:
    explicit BackendDashboard(QWidget *parent = nullptr);
    ~BackendDashboard();
private slots:
    void showCreateTableDialog();
    void showAddFieldDialog();

private:
    Ui::BackendDashboard *ui;
    void applyStylesBack();
    void setupTasksTable();
    void setupTasksMethodsList();
    CreateTableDialog *createTableDialog;
    AddFieldDialog *addFieldDialog;
};

#endif // BACKENDDASHBOARD_H
