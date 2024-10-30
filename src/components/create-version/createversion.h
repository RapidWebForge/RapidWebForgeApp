#ifndef CREATEVERSION_H
#define CREATEVERSION_H

#include <QDialog>

namespace Ui {
class CreateVersion;
}

class CreateVersion : public QDialog
{
    Q_OBJECT

public:
    explicit CreateVersion(QWidget *parent = nullptr);
    ~CreateVersion();

private:
    Ui::CreateVersion *ui;
};

#endif // CREATEVERSION_H
