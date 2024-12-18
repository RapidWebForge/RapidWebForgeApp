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
    void onItemDropped(QTreeWidgetItem *parentItem, QTreeWidgetItem *droppedItem, int dropIndex);
    void onPropertyValueChanged(int row, int column);

    void on_deleteButton_clicked();

private:
    Ui::FrontendDashboard *ui;

    void applyStylesFront();

    void configureTreeWidget(CustomTreeWidget *treeWidget,
                             bool acceptDrops,
                             QAbstractItemView::DragDropMode mode);
    void setUpTreeWidgets();

    void setDraggableFlags(QTreeWidgetItem *item, bool isDraggable);
    void setComponentsDraggable();

    void populateCurrentViewTree();
    void convertTreeToViews();
    void populateNestedItems(QTreeWidgetItem *parentItem,
                             const std::vector<Component> &nestedComponents);
    void populatePropertiesTable(const Component &component);
    Component convertItemToComponent(QTreeWidgetItem *item);
    Component *findComponentByHierarchy(std::vector<Component> &components,
                                        const std::vector<QTreeWidgetItem *> &hierarchy,
                                        int level);
    Component *findComponentInTree(View &view, QTreeWidgetItem *item);
    Component *findNestedComponent(Component &parent, QTreeWidgetItem *item);

    // Auxiliar functions to onItemDropped
    QTreeWidgetItem *createTreeItem(const QString &text);
    void insertComponentInView(Component &newComponent, QTreeWidgetItem *parentItem, int dropIndex);
    void insertNestedComponent(Component *parentComponent,
                               Component &newComponent,
                               QTreeWidgetItem *parentItem,
                               int dropIndex);
    bool isParentView(QTreeWidgetItem *item) const;

    CreateView *createViewDialog;
    std::vector<Route> routes;
    std::vector<View> views;
    View currentView;
    Component currentComponent;
};

#endif // FRONTENDDASHBOARD_H
