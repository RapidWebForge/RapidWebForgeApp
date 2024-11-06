#include "frontenddashboard.h"
#include <QDebug>
#include <QDropEvent>
#include <QMessageBox>
#include "../../models/component-type/componenttype.h"
#include "ui_frontenddashboard.h"

FrontendDashboard::FrontendDashboard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FrontendDashboard)
    , createViewDialog(nullptr)
{
    ui->setupUi(this);

    connect(ui->currentViewTree, &CustomTreeWidget::itemDropped, this, &FrontendDashboard::onItemDropped);
    connect(ui->addSectionButton,
            &QPushButton::clicked,
            this,
            &FrontendDashboard::showCreateViewDialog);
    connect(ui->currentViewTree,
            &QTreeWidget::itemClicked,
            this,
            &FrontendDashboard::onCurrentViewTreeItemSelected);
    connect(ui->propertiesTable,
            &QTableWidget::cellChanged,
            this,
            &FrontendDashboard::onPropertyValueChanged);

    applyStylesFront();
    setUpTreeWidgets();
}

FrontendDashboard::~FrontendDashboard()
{
    delete ui;
}

void FrontendDashboard::applyStylesFront()
{
    this->setStyleSheet("QTreeWidget {"
                        "   border: 1px solid #eaeaea;"
                        "   border-radius: 8px;"
                        "   background-color: white;"
                        "   font-size: 14px;"
                        "   margin: 10px;"
                        "   padding: 5px;"
                        "}"
                        "QTreeWidget::item {"
                        "   border-radius: 5px;"
                        "   margin: 2px;"
                        "   padding: 5px;"
                        "}"
                        "QTreeWidget::item:selected {"
                        "   background-color: #0F66DE;"
                        "   color: white;"
                        "}"
                        "QGroupBox {"
                        "   border: 1px solid #dcdcdc;"
                        "   border-radius: 8px;"
                        "   margin-top: 15px;"
                        "   padding: 10px;"
                        "   font-size: 16px;"
                        "   color: #333;"
                        "}"
                        "QTreeWidget, QTableWidget, QListWidget {"
                        "   border: 1px solid #eaeaea;"
                        "   border-radius: 5px;"
                        "   font-size: 14px;"
                        "   background-color: #fafafa;"
                        "}"
                        "QPushButton {"
                        "   background-color: white;"
                        "   color: white;"
                        "   border: none;"
                        "   border-radius: 12px;"
                        "   padding: 5px 5px;"
                        "   font-size: 12px;"
                        "}"
                        "QPushButton:hover {"
                        "   background-color: #eaeaea;"
                        "   color: white;"
                        "}"
                        "QPushButton:pressed {"
                        "   background-color: #eaeaea;"
                        "}"
                        "QLabel#titleLabel {"
                        "   font-size: 36px;"
                        "   font-weight: bold;"
                        "   color: #333;"
                        "}"
                        "QGroupBox {"
                        "   border: 1px solid #dcdcdc;"
                        "   border-radius: 8px;"
                        "   padding: 10px;"
                        "   font-size: 16px;"
                        "   color: #333;"
                        "}");

    ui->titleLabel->setStyleSheet("font-size: 35px; color: #27292A; padding-top: 10px; "
                                  "padding-left: 40px; padding-bottom: 20px;");
}

void FrontendDashboard::setUpTreeWidgets()
{
    // Components Tree (source)
    CustomTreeWidget *componentsTree = ui->componentsTree;
    componentsTree->setDragEnabled(true);
    componentsTree->setDragDropMode(QAbstractItemView::DragOnly);
    componentsTree->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // Current View Tree (target)
    CustomTreeWidget *currentViewTree = ui->currentViewTree;
    currentViewTree->setAcceptDrops(true);
    currentViewTree->setDragDropMode(QAbstractItemView::DropOnly);
    currentViewTree->setSelectionMode(QAbstractItemView::SingleSelection);
    currentViewTree->setDropIndicatorShown(true);
    
    setComponentsDraggable();
}

void FrontendDashboard::setComponentsDraggable()
{
    int topLevelItemCount = ui->componentsTree->topLevelItemCount();

    for (int i = 0; i < topLevelItemCount; ++i) {
        QTreeWidgetItem *groupItem = ui->componentsTree->topLevelItem(i);

        // Deshabilita el arrastre para el grupo de nivel superior
        if (groupItem) {
            groupItem->setFlags(groupItem->flags() & ~Qt::ItemIsDragEnabled);
        }

        // Habilita el arrastre para los hijos del grupo
        for (int j = 0; j < groupItem->childCount(); ++j) {
            QTreeWidgetItem *childItem = groupItem->child(j);
            if (childItem) {
                childItem->setFlags(childItem->flags() | Qt::ItemIsDragEnabled);
            }
        }
    }
}

void FrontendDashboard::onItemDropped(QTreeWidgetItem *parentItem,
                                      QTreeWidgetItem *droppedItem,
                                      int dropIndex)
{
    qDebug() << "onItemDropped called!";
    qDebug() << "Parent:" << (parentItem ? parentItem->text(0) : "null");
    qDebug() << "Dropped:" << (droppedItem ? droppedItem->text(0) : "null");

    if (!parentItem || !droppedItem) {
        qDebug() << "Error: null items in onItemDropped";
        return;
    }

    // Obtener el tipo de componente del elemento padre
    std::string parentComponentTypeStr = parentItem->text(0).toStdString();
    bool isView = false;

    for (auto &v : views) {
        if (v.getName() == parentComponentTypeStr)
            isView = true;
    }

    if (isView) {
        std::vector<Component> _components = currentView.getComponents();

        Component newComponent = convertItemToComponent(droppedItem);

        _components.push_back(newComponent);

        currentView.setComponents(_components);

        QTreeWidgetItem *newChildItem = new QTreeWidgetItem(parentItem);
        newChildItem->setText(0, droppedItem->text(0));

        parentItem->insertChild(dropIndex, newChildItem);

        // parentItem->addChild(newChildItem);
    } else {
        Component *parentComponent = findComponentInTree(currentView, parentItem);

        if (!parentComponent) {
            QMessageBox::warning(this, "Component error", "Parent wasn't found.");
            return;
        }

        // Verificar si el componente padre permite anidar
        if (!parentComponent->isAllowingItems()) {
            QMessageBox::warning(this,
                                 "Invalid Operation",
                                 "This component does not allow nesting.");
            return;
        }

        // Convertir droppedItem a un nuevo componente
        Component newComponent = convertItemToComponent(droppedItem);

        // Agregar el nuevo componente como componente anidado al padre
        parentComponent->addNestedComponent(newComponent);

        // Actualizar visualmente en el QTreeWidget
        QTreeWidgetItem *newChildItem = new QTreeWidgetItem(parentItem);
        newChildItem->setText(0, droppedItem->text(0));
        // Configura el texto u otras propiedades según sea necesario
        parentItem->addChild(newChildItem);

        // Expandir el elemento padre para ver el nuevo componente
        parentItem->setExpanded(true);

        // Confirmación opcional con qDebug
        qDebug() << "Component dropped and nested under parent component:";
        qDebug() << "  Parent Component Type:"
                 << QString::fromStdString(componentTypeToString(parentComponent->getType()));
        qDebug() << "  Dropped Component Type:"
                 << QString::fromStdString(componentTypeToString(newComponent.getType()));
    }
}

void FrontendDashboard::populateCurrentViewTree()
{
    ui->currentViewTree->clear();

    // Iterar sobre todas las vistas (suponiendo que tienes un vector de vistas llamado `views`)
    for (const auto &view : views) {
        // Añadir cada vista como un item de primer nivel
        QTreeWidgetItem *viewItem = new QTreeWidgetItem(ui->currentViewTree);
        viewItem->setText(0,
                          QString::fromStdString(
                              view.getName())); // Suponiendo que la vista tiene un nombre

        // Añadir los componentes de la vista actual
        for (const auto &component : view.getComponents()) {
            QTreeWidgetItem *componentItem = new QTreeWidgetItem(viewItem);
            componentItem->setText(0,
                                   QString::fromStdString(
                                       componentTypeToString(component.getType())));

            // Verificar si el componente permite anidar y tiene subcomponentes
            if (component.isAllowingItems() && !component.getNestedComponents().empty()) {
                populateNestedItems(componentItem, component.getNestedComponents());
            }
        }
    }

    ui->currentViewTree->expandAll();
}

void FrontendDashboard::populateNestedItems(QTreeWidgetItem *parentItem,
                                            const std::vector<Component> &nestedComponents)
{
    for (const auto &subcomponent : nestedComponents) {
        QTreeWidgetItem *subItem = new QTreeWidgetItem(parentItem);
        subItem->setText(0, QString::fromStdString(componentTypeToString(subcomponent.getType())));

        if (subcomponent.isAllowingItems() && !subcomponent.getNestedComponents().empty()) {
            populateNestedItems(subItem, subcomponent.getNestedComponents());
        }
    }
}

void FrontendDashboard::convertTreeToViews()
{
    std::string viewName = currentView.getName();
    auto it = std::find_if(views.begin(), views.end(), [&viewName](const View &v) {
        return v.getName() == viewName;
    });

    if (it != views.end()) {
        *it = currentView;
    }

    // for (const auto &component : it->getComponents()) {
    //     qDebug() << "  Component Type:"
    //              << QString::fromStdString(componentTypeToString(component.getType()));
    //     for (const auto &prop : component.getProps()) {
    //         qDebug() << "     Prop:" << QString::fromStdString(prop.first) << "="
    //                  << QString::fromStdString(prop.second);
    //     }

    //     if (component.isAllowingItems()) {
    //         for (const auto &nestedComponent : component.getNestedComponents()) {
    //             qDebug() << "     Nested Component Type:"
    //                      << QString::fromStdString(
    //                             componentTypeToString(nestedComponent.getType()));
    //             for (const auto &nestedProp : nestedComponent.getProps()) {
    //                 qDebug() << "        Nested Prop:"
    //                          << QString::fromStdString(nestedProp.first) << "="
    //                          << QString::fromStdString(nestedProp.second);
    //             }
    //         }
    //     }
    // }
}

Component FrontendDashboard::convertItemToComponent(QTreeWidgetItem *item)
{
    std::string componentTypeStr = item->text(0).toStdString();
    ComponentType componentType = stringToComponentType(componentTypeStr);
    Component newComponent(componentType);

    // Verificar si el componente permite anidación y tiene hijos
    if (item->childCount() > 0 && newComponent.isAllowingItems()) {
        std::vector<Component> nestedComponents;
        for (int i = 0; i < item->childCount(); ++i) {
            QTreeWidgetItem *childItem = item->child(i);
            nestedComponents.push_back(
                convertItemToComponent(childItem)); // Conversión recursiva para anidamiento
        }
        newComponent.setNestedComponents(nestedComponents);
    }

    return newComponent;
}

void FrontendDashboard::on_saveButton_clicked()
{
    convertTreeToViews();

    QMessageBox::information(this, "Successful", "View updated");
}

void FrontendDashboard::onCurrentViewTreeItemSelected(QTreeWidgetItem *item, int column)
{
    if (!item) {
        qDebug() << "Selected item is null.";
        return;
    }

    // Usamos una pila de QTreeWidgetItem para guardar la jerarquía completa hasta el item actual
    std::vector<QTreeWidgetItem *> hierarchy;
    QTreeWidgetItem *currentItem = item;
    while (currentItem) {
        hierarchy.insert(hierarchy.begin(), currentItem);
        currentItem = currentItem->parent();
    }

    // Buscar el componente en el currentView usando la jerarquía de items
    Component *foundComponent = findComponentByHierarchy(currentView.getComponents(), hierarchy, 1);

    if (foundComponent) {
        currentComponent = *foundComponent;
        qDebug() << "Found Component:"
                 << QString::fromStdString(componentTypeToString(currentComponent.getType()));

        // Llenar la tabla de propiedades con las propiedades del componente
        populatePropertiesTable(currentComponent);
    } else {
        qDebug() << "Component of type" << QString::fromStdString(item->text(0).toStdString())
                 << "not found.";
    }
}

// Función recursiva que busca el componente utilizando la jerarquía
Component *FrontendDashboard::findComponentByHierarchy(
    std::vector<Component> &components, const std::vector<QTreeWidgetItem *> &hierarchy, int level)
{
    if (level >= hierarchy.size()) {
        return nullptr;
    }

    const std::string type = hierarchy[level]->text(0).toStdString();

    for (auto &component : components) {
        if (componentTypeToString(component.getType()) == type) {
            // Si estamos en el último nivel de la jerarquía, devolvemos el componente encontrado
            if (level == hierarchy.size() - 1) {
                return &component;
            }

            // Si hay más niveles, continuamos buscando en los nestedComponents
            if (component.isAllowingItems()) {
                Component *nested = findComponentByHierarchy(component.getNestedComponents(),
                                                             hierarchy,
                                                             level + 1);
                if (nested) {
                    return nested;
                }
            }
        }
    }

    return nullptr;
}

void FrontendDashboard::populatePropertiesTable(const Component &component)
{
    // Desconectar temporalmente onPropertyValueChanged para evitar bucles
    disconnect(ui->propertiesTable,
               &QTableWidget::cellChanged,
               this,
               &FrontendDashboard::onPropertyValueChanged);

    // Limpiar la tabla de propiedades y ajustar el número de filas
    ui->propertiesTable->clearContents();
    ui->propertiesTable->setRowCount(0);

    const auto &props = component.getProps();

    if (props.empty()) {
        qDebug() << "No properties found for this component.";
        return;
    }

    // Ajustar el número de filas
    ui->propertiesTable->setRowCount(props.size());

    int row = 0;
    for (const auto &prop : props) {
        QTableWidgetItem *keyItem = new QTableWidgetItem(QString::fromStdString(prop.first));
        QTableWidgetItem *valueItem = new QTableWidgetItem(QString::fromStdString(prop.second));

        keyItem->setFlags(keyItem->flags() & ~Qt::ItemIsEditable);

        ui->propertiesTable->setItem(row, 0, keyItem);
        ui->propertiesTable->setItem(row, 1, valueItem);

        ++row;
    }

    // Reconectar onPropertyValueChanged después de la actualización
    connect(ui->propertiesTable,
            &QTableWidget::cellChanged,
            this,
            &FrontendDashboard::onPropertyValueChanged);
}

void FrontendDashboard::onPropertyValueChanged(int row, int column)
{
    if (column == 1) { // Solo manejar cambios en la columna de valores
        QString propertyName = ui->propertiesTable->item(row, 0)->text();
        QString newValue = ui->propertiesTable->item(row, 1)->text();

        // Actualizar las propiedades en `currentComponent`
        std::map<std::string, std::string> currentProps = currentComponent.getProps();
        currentProps[propertyName.toStdString()] = newValue.toStdString();
        currentComponent.setProps(currentProps);

        // qDebug() << "Updated property" << propertyName << "to" << newValue;

        // Actualizar el componente en `currentView` usando el método `findComponentInTree`
        Component *componentInView = findComponentInTree(currentView,
                                                         ui->currentViewTree->currentItem());

        if (componentInView) {
            componentInView->setProps(currentComponent.getProps());
            qDebug() << "Property updated in currentView. Updated Properties:";
            for (const auto &prop : componentInView->getProps()) {
                qDebug() << "  " << QString::fromStdString(prop.first) << "="
                         << QString::fromStdString(prop.second);
            }
        } else {
            qDebug() << "Could not find component in currentView.";
        }

        // Reflejar el currentView en views
        auto it = std::find_if(views.begin(), views.end(), [this](const View &view) {
            return view.getName() == currentView.getName();
        });

        if (it != views.end()) {
            *it = currentView;

            qDebug() << "Current view updated in views.";

        } else {
            qDebug() << "Could not update current view in views.";
        }
    }
}

Component *FrontendDashboard::findComponentInTree(View &view, QTreeWidgetItem *item)
{
    // Usamos la jerarquía completa de item para realizar una búsqueda exacta
    std::vector<QTreeWidgetItem *> hierarchy;
    QTreeWidgetItem *currentItem = item;

    // Construir la jerarquía desde el item hasta la raíz
    while (currentItem) {
        hierarchy.insert(hierarchy.begin(), currentItem);
        currentItem = currentItem->parent();
    }

    // Comprobamos que el primer nivel en hierarchy es la vista
    if (hierarchy.size() > 0 && hierarchy[0]->text(0).toStdString() == view.getName()) {
        // Ignorar el nivel de la vista y comenzar desde el siguiente
        return findComponentByHierarchy(view.getComponents(), hierarchy, 1);
    } else {
        qDebug() << "Error: La vista no coincide con el nivel superior de hierarchy.";
        return nullptr;
    }
}

Component *FrontendDashboard::findNestedComponent(Component &parent, QTreeWidgetItem *item)
{
    const std::string type = item->text(0).toStdString();
    std::vector<Component> &nestedComponents = parent.getNestedComponents();

    qDebug() << "Searching for nested component of type" << QString::fromStdString(type);

    for (auto &nestedComponent : nestedComponents) {
        qDebug() << "Checking nested component of type"
                 << QString::fromStdString(componentTypeToString(nestedComponent.getType()));

        if (componentTypeToString(nestedComponent.getType()) == type && !item->parent()) {
            qDebug() << "Nested component found.";
            return &nestedComponent;
        }

        // Si el `QTreeWidgetItem` tiene más niveles, sigue buscando en niveles más profundos
        if (nestedComponent.isAllowingItems() && item->parent()) {
            Component *deepNested = findNestedComponent(nestedComponent, item->parent());
            if (deepNested) {
                return deepNested;
            }
        }
    }

    qDebug() << "Nested component of type" << QString::fromStdString(type) << "not found.";
    return nullptr;
}

void FrontendDashboard::showCreateViewDialog()
{
    if (!createViewDialog) {
        createViewDialog = new CreateView(this);

        connect(createViewDialog, &CreateView::routeSaved, this, &FrontendDashboard::onRouteSaved);
    }
    createViewDialog->exec();
}

void FrontendDashboard::onRouteSaved(const Route &route)
{
    routes.push_back(route);

    // Crear una nueva vista si no existe
    // auto it = std::find_if(views.begin(), views.end(), [&route](const View &view) {
    //     return view.getName() == route.getComponent();
    // });

    // Si la vista no existe, crear una nueva y agregarla
    // if (it == views.end()) {
    //     View newView(route.getComponent());
    //     views.push_back(newView);
    // }

    // Actualizar las rutas y vistas en la interfaz
    // setRoutes(routes);
    // setViews(views);
}

// Getters
const std::vector<Route> &FrontendDashboard::getRoutes() const
{
    return routes;
}

std::vector<Route> &FrontendDashboard::getRoutes()
{
    return routes;
}

std::vector<View> &FrontendDashboard::getViews()
{
    return views;
}

const std::vector<View> &FrontendDashboard::getViews() const
{
    return views;
}

// Setters
void FrontendDashboard::setRoutes(const std::vector<Route> &routes)
{
    this->routes = routes;
}

void FrontendDashboard::setViews(const std::vector<View> &views)
{
    this->views = views;
}

void FrontendDashboard::setCurrentView(View &view)
{
    currentView = view;

    populateCurrentViewTree();
}
