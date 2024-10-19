#ifndef FRONTENDDASHBOARD_H
#define FRONTENDDASHBOARD_H

#include <QDialog>
#include <QTreeWidget>
#include "../../models/route/route.h"
#include "../../models/view/view.h"
#include "../create-view/createview.h"
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
    const std::vector<Route> &getRoutes() const;
    std::vector<Route> &getRoutes();
    const std::vector<View> &getViews() const;
    std::vector<View> &getViews();
    // Setters
    void setRoutes(const std::vector<Route> &routes);
    void setViews(const std::vector<View> &views);
    void setCurrentView(View &view);

public slots:
    void onRouteSaved(const Route &route);

private slots:
    void showCreateViewDialog();

private:
    Ui::FrontendDashboard *ui;
    void applyStylesFront();
    void setUpTreeWidgets();
    void setComponentsDraggable();
    void populateCurrentViewTree();
    void convertTreeToViews();
    void populateNestedComponents(QTreeWidgetItem *parentItem, std::vector<Component> &component);

    CreateView *createViewDialog;
    std::vector<Route> routes;
    std::vector<View> views;
    View currentView;
};

#endif // FRONTENDDASHBOARD_H
