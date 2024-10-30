#ifndef VERSIONHISTORY_H
#define VERSIONHISTORY_H

#include <QDialog>

namespace Ui {
class VersionHistory;
}

class VersionHistory : public QDialog
{
    Q_OBJECT

public:
    explicit VersionHistory(QWidget *parent = nullptr);
    ~VersionHistory();

private:
    Ui::VersionHistory *ui;
};

#endif // VERSIONHISTORY_H
