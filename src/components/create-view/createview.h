#ifndef CREATEVIEW_H
#define CREATEVIEW_H

#include <QDialog>
#include "../../models/route/route.h"

namespace Ui {
class CreateView;
}

class CreateView : public QDialog
{
    Q_OBJECT

public:
    explicit CreateView(QWidget *parent = nullptr);
    ~CreateView();

signals:
    void routeSaved(const Route &route);

private slots:
    void on_createButton_clicked();

private:
    Ui::CreateView *ui;
    Route route;
};

#endif // CREATEVIEW_H
