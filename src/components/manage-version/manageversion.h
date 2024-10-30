#ifndef MANAGEVERSION_H
#define MANAGEVERSION_H

#include <QDialog>

namespace Ui {
class ManageVersion;
}

class ManageVersion : public QDialog
{
    Q_OBJECT

public:
    explicit ManageVersion(QWidget *parent = nullptr);
    ~ManageVersion();

private:
    Ui::ManageVersion *ui;
};

#endif // MANAGEVERSION_H
