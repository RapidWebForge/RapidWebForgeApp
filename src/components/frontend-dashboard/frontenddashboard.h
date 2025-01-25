#ifndef FRONTENDDASHBOARD_H
#define FRONTENDDASHBOARD_H

#include <QDialog>
#include <QTreeWidget>
#include "../../models/component/component.h"
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
    const std::vector<std::shared_ptr<Section>> &getViews() const;
    const std::vector<std::shared_ptr<Section>> &getCustomComponents() const;
    // Setters
    void setRoutes(const std::vector<Route> &routes);
    void setViews(const std::vector<std::shared_ptr<Section>> &views);
    void setCustomComponents(const std::vector<std::shared_ptr<Section>> &custComponents);
    void setCurrentSection(const std::shared_ptr<Section> &section);
    // ComboBox Section
    void fillAvailableSections();

public slots:
    void onRouteSaved(const Route &route);
    void onCustomComponentSaved(const std::shared_ptr<Section> &component);

private slots:
    void onCurrentSectionTreeItemSelected(QTreeWidgetItem *item, int column);
    void onItemDropped(QTreeWidgetItem *parentItem, QTreeWidgetItem *droppedItem, int dropIndex);
    void onPropertyValueChanged(int row, int column);

    void on_saveButton_clicked();
    void on_deleteButton_clicked();
    void on_addSectionButton_clicked();
    void on_sectionComboBox_currentIndexChanged(int index);

private:
    Ui::FrontendDashboard *ui;

    void applyStylesFront();
    void configureTreeWidget(CustomTreeWidget *treeWidget,
                             bool acceptDrops,
                             QAbstractItemView::DragDropMode mode);
    void setUpTreeWidgets();

    // Auxiliar functions
    QTreeWidgetItem *createTreeItem(const QString &text,
                                    CustomTreeWidget *treeWidget = nullptr,
                                    QTreeWidgetItem *parentItem = nullptr);
    std::string getComponentIdFromTree(QTreeWidgetItem *item) const;

    void setDraggableFlags(QTreeWidgetItem *item, bool isDraggable);
    void setComponentsDraggable();

    // Populate
    void populateCurrentSectionTree();
    void populateNestedItems(QTreeWidgetItem *parentItem,
                             const std::vector<std::shared_ptr<BaseNode>> &nestedComponents);
    void populateSubSectionItems(QTreeWidgetItem *parentItem,
                                 const std::shared_ptr<Section> &subSection);
    void populatePropertiesTable(const std::shared_ptr<Component> &component);

    std::shared_ptr<BaseNode> convertItemToBaseNode(QTreeWidgetItem *item);

    std::shared_ptr<Component> findComponentInTree(const std::shared_ptr<BaseNode> &view,
                                                   QTreeWidgetItem *item);
    std::shared_ptr<Component> findComponentByHierarchy(
        const std::vector<std::shared_ptr<BaseNode>> &components,
        const std::vector<QTreeWidgetItem *> &hierarchy,
        std::string id,
        int level);
    // Component *findNestedComponent(Component &parent, QTreeWidgetItem *item);

    // Auxiliar functions to onItemDropped
    void insertComponentInSection(std::shared_ptr<BaseNode> &newComponent,
                                  QTreeWidgetItem *parentItem,
                                  int dropIndex);
    void insertNestedComponent(std::shared_ptr<Component> &parentComponent,
                               std::shared_ptr<BaseNode> &newComponent,
                               QTreeWidgetItem *parentItem,
                               int dropIndex);
    bool isView(QTreeWidgetItem *item) const;
    bool isCustomComponent(QTreeWidgetItem *item) const;

    bool deleteComponentByHierarchy(const std::shared_ptr<Section> &section,
                                    const std::vector<QTreeWidgetItem *> &hierarchy);

    CreateSection *createSectionDialog;
    std::vector<Route> routes;
    std::vector<std::shared_ptr<Section>> custComponents;
    std::vector<std::shared_ptr<Section>> views;
    std::shared_ptr<Section> currentSection;
    std::shared_ptr<Component> currentComponent;
};

#endif // FRONTENDDASHBOARD_H
