#include "frontenddashboard.h"
#include <QDebug>
#include <QDropEvent>
#include <QFile>
#include <QMessageBox>
#include "../../models/component-type/componenttype.h"
#include "ui_frontenddashboard.h"

FrontendDashboard::FrontendDashboard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FrontendDashboard)
    , createViewDialog(nullptr)
{
    ui->setupUi(this);

    connect(ui->currentViewTree,
            &CustomTreeWidget::itemDropped,
            this,
            &FrontendDashboard::onItemDropped);
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
    setComponentsDraggable();
}

FrontendDashboard::~FrontendDashboard()
{
    delete ui;
}

void FrontendDashboard::applyStylesFront()
{
    QFile styleFile(":/styles/frontenddashboard");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        this->setStyleSheet(styleSheet);
    }

    ui->titleLabel->setStyleSheet("font-size: 35px; color: #27292A; padding-top: 10px; "
                                  "padding-left: 40px; padding-bottom: 20px;");
}

// TreeWidgets config

void FrontendDashboard::configureTreeWidget(CustomTreeWidget *treeWidget,
                                            bool acceptDrops,
                                            QAbstractItemView::DragDropMode mode)
{
    treeWidget->setDragEnabled(mode != QAbstractItemView::DropOnly);
    treeWidget->setAcceptDrops(acceptDrops);
    treeWidget->setDragDropMode(mode);
    treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    treeWidget->setDropIndicatorShown(true);
}

void FrontendDashboard::setUpTreeWidgets()
{
    configureTreeWidget(ui->componentsTree, false, QAbstractItemView::DragOnly);
    configureTreeWidget(ui->currentViewTree, true, QAbstractItemView::DropOnly);
}

// Draggable settings

void FrontendDashboard::setDraggableFlags(QTreeWidgetItem *item, bool isDraggable)
{
    if (isDraggable) {
        item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    } else {
        item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled);
    }
}

void FrontendDashboard::setComponentsDraggable()
{
    int topLevelItemCount = ui->componentsTree->topLevelItemCount();
    for (int i = 0; i < topLevelItemCount; ++i) {
        QTreeWidgetItem *groupItem = ui->componentsTree->topLevelItem(i);
        if (groupItem) {
            setDraggableFlags(groupItem, false); // No arrastrar grupos
            for (int j = 0; j < groupItem->childCount(); ++j) {
                setDraggableFlags(groupItem->child(j), true); // Arrastrar solo hijos
            }
        }
    }
}

// Auxiliar functions to onItemDropped

QTreeWidgetItem *FrontendDashboard::createTreeItem(const QString &text)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, text);
    return item;
}

void FrontendDashboard::insertComponentInView(Component &newComponent,
                                              QTreeWidgetItem *parentItem,
                                              int dropIndex)
{
    std::vector<Component> &components = currentView.getComponents();
    dropIndex = std::clamp(dropIndex, 0, static_cast<int>(components.size()));
    components.insert(components.begin() + dropIndex, newComponent);

    QTreeWidgetItem *newItem = createTreeItem(
        QString::fromStdString(componentTypeToString(newComponent.getType())));
    if (parentItem == ui->currentViewTree->invisibleRootItem()) {
        ui->currentViewTree->insertTopLevelItem(dropIndex, newItem);
    } else {
        parentItem->insertChild(dropIndex, newItem);
    }
}

void FrontendDashboard::insertNestedComponent(Component *parentComponent,
                                              Component &newComponent,
                                              QTreeWidgetItem *parentItem,
                                              int dropIndex)
{
    if (!parentComponent->isAllowingItems()) {
        QMessageBox::warning(this, "Invalid Operation", "This component does not allow nesting.");
        return;
    }
    std::vector<Component> &nestedComponents = parentComponent->getNestedComponents();
    dropIndex = std::clamp(dropIndex, 0, static_cast<int>(nestedComponents.size()));
    nestedComponents.insert(nestedComponents.begin() + dropIndex, newComponent);

    QTreeWidgetItem *newItem = createTreeItem(
        QString::fromStdString(componentTypeToString(newComponent.getType())));
    parentItem->insertChild(dropIndex, newItem);
}

// End of Auxiliar functions to onItemDropped

bool FrontendDashboard::isParentView(QTreeWidgetItem *item) const
{
    std::string itemType = item->text(0).toStdString();
    return std::any_of(views.begin(), views.end(), [&](const auto &v) {
        return v.getName() == itemType;
    });
}

void FrontendDashboard::onItemDropped(QTreeWidgetItem *parentItem,
                                      QTreeWidgetItem *droppedItem,
                                      int dropIndex)
{
    if (!parentItem || !droppedItem) {
        qDebug() << "Error: null items in onItemDropped";
        return;
    }

    bool isView = isParentView(parentItem);

    Component newComponent = convertItemToComponent(droppedItem);

    if (isView) {
        insertComponentInView(newComponent, parentItem, dropIndex);
    } else {
        Component *parentComponent = findComponentInTree(currentView, parentItem);

        if (!parentComponent) {
            QMessageBox::warning(this, "Component error", "Parent wasn't found.");
            return;
        }

        insertNestedComponent(parentComponent, newComponent, parentItem, dropIndex);
    }
}

void FrontendDashboard::populateCurrentViewTree()
{
    ui->currentViewTree->clear();

    // Iterar sobre todas las vistas
    for (const auto &view : views) {
        // Añadir cada vista como un item de primer nivel
        QTreeWidgetItem *viewItem = new QTreeWidgetItem(ui->currentViewTree);
        viewItem->setText(0, QString::fromStdString(view.getName()));

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

    // Obtener el nombre del elemento seleccionado
    std::string selectedItemName = item->text(0).toStdString();

    // Verificar si el elemento seleccionado es una vista
    auto viewIt = std::find_if(views.begin(), views.end(), [&selectedItemName](const View &view) {
        return view.getName() == selectedItemName;
    });

    if (viewIt != views.end()) {
        // Si el elemento seleccionado es una vista, actualizar `currentView`
        currentView = *viewIt;
        qDebug() << "Current view updated to:" << QString::fromStdString(currentView.getName());
        return;
    }

    // Si el elemento seleccionado no es una vista, buscar el componente en `currentView`

    // Usamos una pila de QTreeWidgetItem para guardar la jerarquía completa hasta el item actual
    std::vector<QTreeWidgetItem *> hierarchy;
    QTreeWidgetItem *currentItem = item;
    while (currentItem) {
        hierarchy.insert(hierarchy.begin(), currentItem);
        currentItem = currentItem->parent();
    }

    // Buscar el componente en `currentView` usando la jerarquía de items
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
    // Agregar la ruta a `routes`
    routes.push_back(route);

    // Crear un nuevo QTreeWidgetItem para la vista y agregarlo al `currentViewTree`
    QTreeWidgetItem *viewItem = new QTreeWidgetItem(ui->currentViewTree);
    viewItem->setText(0, QString::fromStdString(route.getComponent()));
    ui->currentViewTree->addTopLevelItem(viewItem);

    // Crear una nueva vista y un componente `Header H1` predeterminado
    View view(route.getComponent());
    Component headerComponent(ComponentType::HeaderH1);

    // Asignar propiedades predeterminadas al `Header H1`
    headerComponent.setProps({{"class", ""}, {"text", "Default Header"}});

    // Agregar `Header H1` a los componentes de la vista
    view.getComponents().push_back(headerComponent);
    view.setComponents(view.getComponents());

    // Sincronizar la vista con `currentView` y `views`
    if (route.getComponent() == currentView.getName()) {
        currentView = view;
    }
    views.push_back(view);

    // Asociar el `QTreeWidgetItem` con la vista para facilitar la búsqueda en drag and drop
    viewItem->setData(0, Qt::UserRole, QVariant::fromValue(&view));

    // Agregar `Header H1` como un elemento hijo en `currentViewTree` bajo el elemento de la vista
    QTreeWidgetItem *headerItem = new QTreeWidgetItem(viewItem);
    headerItem->setText(0, "Header H1");

    // Asociar `headerItem` con el componente `headerComponent`
    headerItem->setData(0, Qt::UserRole, QVariant::fromValue(&headerComponent));

    // Expandir el elemento de la vista para mostrar `Header H1`
    viewItem->setExpanded(true);

    // Actualizar las rutas y vistas en la interfaz si es necesario
    setRoutes(routes);
}

void FrontendDashboard::on_deleteButton_clicked()
{
    // Obtener el elemento seleccionado en `currentViewTree`
    QTreeWidgetItem *selectedItem = ui->currentViewTree->currentItem();
    if (!selectedItem) {
        qDebug() << "No item selected for deletion.";
        return;
    }

    // Obtener el nombre del elemento seleccionado
    std::string selectedItemName = selectedItem->text(0).toStdString();

    // Verificar si el elemento seleccionado es una vista
    auto viewIt = std::find_if(views.begin(), views.end(), [&selectedItemName](const View &view) {
        return view.getName() == selectedItemName;
    });

    if (viewIt != views.end()) {
        // Si es una vista, eliminar la vista de `views` y de `currentViewTree`
        views.erase(viewIt);
        delete selectedItem; // Esto eliminará el elemento del árbol
        qDebug() << "View deleted:" << QString::fromStdString(selectedItemName);
    } else {
        // Si no es una vista, buscar y eliminar el componente en `currentView`

        // Usamos una pila de QTreeWidgetItem para guardar la jerarquía completa hasta el item actual
        std::vector<QTreeWidgetItem *> hierarchy;
        QTreeWidgetItem *currentItem = selectedItem;
        while (currentItem) {
            hierarchy.insert(hierarchy.begin(), currentItem);
            currentItem = currentItem->parent();
        }

        // Buscar el componente en `currentView` usando la jerarquía de items
        Component *foundComponent = findComponentByHierarchy(currentView.getComponents(),
                                                             hierarchy,
                                                             1);

        if (foundComponent) {
            // Eliminar el componente de `currentView`
            auto &components = currentView.getComponents();
            components.erase(std::remove_if(components.begin(),
                                            components.end(),
                                            [foundComponent](const Component &component) {
                                                return &component == foundComponent;
                                            }),
                             components.end());

            // Eliminar el elemento del árbol
            delete selectedItem;
            qDebug() << "Component deleted:"
                     << QString::fromStdString(componentTypeToString(foundComponent->getType()));
        } else {
            qDebug() << "Component not found for deletion.";
        }
    }
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
