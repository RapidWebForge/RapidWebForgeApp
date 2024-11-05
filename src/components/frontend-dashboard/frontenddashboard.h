#ifndef FRONTENDDASHBOARD_H
#define FRONTENDDASHBOARD_H

#include <QDialog>
#include <QTreeWidget>
#include "../../models/route/route.h"
#include "../../models/view/view.h"
#include "../create-view/createview.h"
#include "../custom-tree-widget/customtreewidget.h"
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

    void on_saveButton_clicked();
    void onCurrentViewTreeItemSelected(QTreeWidgetItem *item, int column);
    void onItemDropped(QTreeWidgetItem *parentItem, QTreeWidgetItem *droppedItem);
    void onPropertyValueChanged(int row, int column);

private:
    Ui::FrontendDashboard *ui;

    void applyStylesFront();
    void setUpTreeWidgets();
    void setComponentsDraggable();
    void populateCurrentViewTree();
    void convertTreeToViews();
    void populateNestedItems(QTreeWidgetItem *parentItem,
                             const std::vector<Component> &nestedComponents);
    void populatePropertiesTable(const Component &component);
    Component convertItemToComponent(QTreeWidgetItem *item);

    CreateView *createViewDialog;
    std::vector<Route> routes;
    std::vector<View> views;
    View currentView;
    Component currentComponent;
};

#endif // FRONTENDDASHBOARD_H
