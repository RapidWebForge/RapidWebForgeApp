#ifndef FRONTENDDASHBOARD_H
#define FRONTENDDASHBOARD_H

#include <QDialog>
#include "../../models/view/view.h"
#include <vector>

namespace Ui {
class FrontendDashboard;
}

class FrontendDashboard : public QDialog
{
    Q_OBJECT

public:
    explicit FrontendDashboard(QWidget *parent = nullptr);
    ~FrontendDashboard();
    // Getters
    const std::vector<View> &getViews() const;
    std::vector<View> &getViews();
    // Setters
    void setViews(const std::vector<View> &views);
    void setCurrentView(View &view);

private slots:
    void on_addSectionButton_clicked();

private:
    Ui::FrontendDashboard *ui;
    void applyStylesFront();

    std::vector<View> views;
    View currentView;
};

#endif // FRONTENDDASHBOARD_H
