#ifndef FRONTENDDASHBOARD_H
#define FRONTENDDASHBOARD_H

#include <QTableWidget>
#include <QTreeWidget>
#include <QWidget>
#include "../../core/logging/actionloggerjson.h" // Para manejar logs en formato .json
#include "../../models/component/component.h"
#include "../../models/section/section.h"
#include "../create-section/createsection.h"
#include "../custom-tree-widget/customtreewidget.h"
#include <vector>

namespace Ui {
class FrontendDashboard;
}

class FrontendDashboard : public QWidget
{
    Q_OBJECT

public:
    explicit FrontendDashboard(QWidget *parent = nullptr);
    ~FrontendDashboard();

    // Getters
    const std::shared_ptr<BaseNode> &getFrontendRoot() const;
    // Setters
    void setFrontendRoot(const std::shared_ptr<BaseNode> &frontendRoot);
    void setCurrentSection(const std::shared_ptr<BaseNode> &section);
    // ComboBox Section
    void fillAvailableSections();
    // Custom Components on Components Tree
    void addCustomComponentsOnComponentsTree();

public slots:
    void onSectionSaved(const std::shared_ptr<Section> &section);

private slots:
    void onCurrentSectionTreeItemSelected(QTreeWidgetItem *item, int column);
    void onItemDropped(QTreeWidgetItem *parentItem, QTreeWidgetItem *droppedItem, int dropIndex);
    void onPropertyValueChanged(int row, int column);
    void onPropertyComboBoxChanged(int row,
                                   const std::string &propertyName,
                                   const QString &newValue);

    // void on_saveButton_clicked();
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
    void cleanPropertiesTable();

    void setDraggableFlags(QTreeWidgetItem *item, bool isDraggable);
    void setComponentsDraggable();

    const std::shared_ptr<BaseNode> getMainNode(const std::string &nodeName) const;

    // Populate
    void populateCurrentSectionTree();
    void populateNestedItems(QTreeWidgetItem *parentItem,
                             const std::vector<std::shared_ptr<BaseNode>> &nestedComponents);
    void populatePropertiesTable(const std::shared_ptr<Component> &component);

    std::shared_ptr<BaseNode> convertItemToBaseNode(QTreeWidgetItem *item);

    std::shared_ptr<Component> findComponentInTree(const std::shared_ptr<BaseNode> &view,
                                                   QTreeWidgetItem *item);
    std::shared_ptr<Component> findComponentByHierarchy(
        const std::vector<std::shared_ptr<BaseNode>> &components,
        const std::vector<QTreeWidgetItem *> &hierarchy,
        const std::string &id,
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

    // Funciones Auxiliares para on_deleteButton_clicked
    void removeSubsectionsOnAST(std::shared_ptr<BaseNode> &node, const std::string &sectionName);
    bool deleteComponentByHierarchy(const std::shared_ptr<BaseNode> &parent,
                                    const std::vector<QTreeWidgetItem *> &hierarchy);

    CreateSection *createSectionDialog;
    std::shared_ptr<BaseNode> frontendRoot;
    std::shared_ptr<Section> currentSection;
    std::shared_ptr<Component> currentComponent;
    ActionLoggerJson loggerJson;
};

#endif // FRONTENDDASHBOARD_H
