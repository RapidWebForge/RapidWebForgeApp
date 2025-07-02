#ifndef CREATESECTION_H
#define CREATESECTION_H

#include <QDialog>
#include "../../core/logging/actionloggerjson.h"
#include "../../models/section/section.h"

namespace Ui {
class CreateSection;
}

class CreateSection : public QDialog
{
    Q_OBJECT

public:
    explicit CreateSection(QWidget *parent = nullptr);
    ~CreateSection();

signals:
    void onSectionSaved(const std::shared_ptr<Section> &newSection);

private slots:
    void on_createButton_clicked();

    void on_cancelButton_clicked();

    void on_isViewCheckBox_checkStateChanged(const Qt::CheckState &arg1);

private:
    Ui::CreateSection *ui;
    ActionLoggerJson loggerJson; // Logs en formato .json

    void applyStyles();
    void resetFields();
};

#endif // CREATESECTION_H
