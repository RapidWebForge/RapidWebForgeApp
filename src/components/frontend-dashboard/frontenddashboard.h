#ifndef FRONTENDDASHBOARD_H
#define FRONTENDDASHBOARD_H

#include <QDialog>
#include <QTreeWidget>
#include "../../models/route/route.h"
#include "../../models/section/section.h"
#include "../create-section/createsection.h"
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
    const std::vector<Section> &getViews() const;
    std::vector<Section> &getViews();
    const std::vector<Section> &getCustomComponents() const;
    std::vector<Section> &getCustomComponents();
    // Setters
    void setRoutes(const std::vector<Route> &routes);
    void setViews(const std::vector<Section> &views);
    void setCustomComponents(const std::vector<Section> &custComponents);
    void setCurrentView(Section &view);

public slots:
    void onRouteSaved(const Route &route);
    void onCustomComponentSaved(const Section &component);

private slots:
    void showCreateSectionDialog();

    void on_saveButton_clicked();
    void onCurrentViewTreeItemSelected(QTreeWidgetItem *item, int column);
    void onItemDropped(QTreeWidgetItem *parentItem, QTreeWidgetItem *droppedItem, int dropIndex);
    void onPropertyValueChanged(int row, int column);

    void on_deleteButton_clicked();
    void on_addSectionButton_clicked();

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
    Component *findComponentInTree(Section &view, QTreeWidgetItem *item);
    Component *findNestedComponent(Component &parent, QTreeWidgetItem *item);

    // Auxiliar functions to onItemDropped
    QTreeWidgetItem *createTreeItem(const QString &text);
    void insertComponentInView(Component &newComponent, QTreeWidgetItem *parentItem, int dropIndex);
    void insertNestedComponent(Component *parentComponent,
                               Component &newComponent,
                               QTreeWidgetItem *parentItem,
                               int dropIndex);
    bool isParentView(QTreeWidgetItem *item) const;

    CreateSection *createSectionDialog;
    std::vector<Route> routes;
    std::vector<Section> custComponents;
    std::vector<Section> views;
    Section currentView;
    Component currentComponent;
};

#endif // FRONTENDDASHBOARD_H
