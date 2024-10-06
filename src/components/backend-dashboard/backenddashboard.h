#ifndef BACKENDDASHBOARD_H
#define BACKENDDASHBOARD_H

#include <QDialog>

namespace Ui {
class BackendDashboard;
}

class BackendDashboard : public QDialog
{
    Q_OBJECT

public:
    explicit BackendDashboard(QWidget *parent = nullptr);
    ~BackendDashboard();

private:
    Ui::BackendDashboard *ui;
};

#endif // BACKENDDASHBOARD_H
