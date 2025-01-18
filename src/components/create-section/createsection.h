#ifndef CREATESECTION_H
#define CREATESECTION_H

#include <QDialog>
#include "../../models/route/route.h"
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
    void routeSaved(const Route &route);
    void customComponentSaved(const Section &custComponent);

private slots:
    void on_createButton_clicked();

    void on_cancelButton_clicked();

    void on_isViewCheckBox_checkStateChanged(const Qt::CheckState &arg1);

private:
    Ui::CreateSection *ui;
    Route route;
    Section custComponent;

    void applyStyles();
};

#endif // CREATESECTION_H
