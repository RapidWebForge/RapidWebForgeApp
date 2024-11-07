#ifndef DELETEVERSION_H
#define DELETEVERSION_H

#include <QDialog>
#include <QStandardItemModel>

namespace Ui {
class DeleteVersion;
}

class DeleteVersion : public QDialog
{
    Q_OBJECT

public:
    explicit DeleteVersion(QWidget *parent = nullptr);
    ~DeleteVersion();

    void setVersions(const std::vector<std::string> &versions);
    QString getSelectedVersion() const;

private:
    Ui::DeleteVersion *ui;
    QStandardItemModel *model;
};

#endif // DELETEVERSION_H
