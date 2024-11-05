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
    // Components Tree
    QTreeWidget *componentsTree = ui->componentsTree;
    componentsTree->setDragEnabled(true);
    componentsTree->setDropIndicatorShown(true);
    componentsTree->setDragDropMode(QAbstractItemView::InternalMove);

    setComponentsDraggable();

    // Current View Tree
    QTreeWidget *currentViewTree = ui->currentViewTree;
    currentViewTree->setAcceptDrops(true);
}

void FrontendDashboard::setComponentsDraggable()
{
    QTreeWidgetItem *inputsGroup = ui->componentsTree->topLevelItem(0); // Primer grupo, "Inputs"
    QTreeWidgetItem *labelsGroup = ui->componentsTree->topLevelItem(1); // Segundo grupo, "Labels"

    // Deshabilitar arrastre para el grupo "Inputs"
    if (inputsGroup) {
        inputsGroup->setFlags(inputsGroup->flags() & ~Qt::ItemIsDragEnabled);
    }

    // Deshabilitar arrastre para el grupo "Labels"
    if (labelsGroup) {
        labelsGroup->setFlags(labelsGroup->flags() & ~Qt::ItemIsDragEnabled);
    }

    // Habilitar arrastre de los hijos
    for (int i = 0; i < inputsGroup->childCount(); ++i) {
        QTreeWidgetItem *child = inputsGroup->child(i);
        child->setFlags(child->flags() | Qt::ItemIsDragEnabled);
    }

    for (int i = 0; i < labelsGroup->childCount(); ++i) {
        QTreeWidgetItem *child = labelsGroup->child(i);
        child->setFlags(child->flags() | Qt::ItemIsDragEnabled);
    }
}

void FrontendDashboard::onItemDropped(QTreeWidgetItem *parentItem, QTreeWidgetItem *droppedItem)
{
    std::string parentComponentTypeStr = parentItem->text(0).toStdString();
    ComponentType parentComponentType = stringToComponentType(parentComponentTypeStr);

    // Verificar si el componente padre permite anidar
    Component parentComponent(parentComponentType);
    if (!parentComponent.isAllowingItems()) {
        QMessageBox::warning(this, "Invalid Operation", "This component does not allow nesting.");
        return;
    }
}

void FrontendDashboard::populateCurrentViewTree()
{
    ui->currentViewTree->clear();

    if (currentView.getComponents().empty()) {
        qDebug() << "Nothing found!" << "\n";
        return;
    }

    for (const auto &component : currentView.getComponents()) {
        QTreeWidgetItem *componentItem = new QTreeWidgetItem(ui->currentViewTree);
        componentItem->setText(0,
                               QString::fromStdString(componentTypeToString(component.getType())));

        // Verificar si el componente permite anidar
        if (component.isAllowingItems() && !component.getNestedComponents().empty()) {
            populateNestedItems(componentItem, component.getNestedComponents());
        }
    }

    ui->currentViewTree->expandAll();
}

void FrontendDashboard::populateNestedItems(QTreeWidgetItem *parentItem,
                                            const std::vector<Component> &nestedComponents)
{
    for (const auto &nestedComponent : nestedComponents) {
        QTreeWidgetItem *childItem = new QTreeWidgetItem(parentItem);
        childItem->setText(0,
                           QString::fromStdString(componentTypeToString(nestedComponent.getType())));

        // Llamada recursiva si el componente tiene componentes anidados
        if (nestedComponent.isAllowingItems() && !nestedComponent.getNestedComponents().empty()) {
            populateNestedItems(childItem, nestedComponent.getNestedComponents());
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
        std::vector<Component> newComponents;

        for (int i = 0; i < ui->currentViewTree->topLevelItemCount(); ++i) {
            QTreeWidgetItem *componentItem = ui->currentViewTree->topLevelItem(i);
            Component newComponent = convertItemToComponent(componentItem);

            // Debugging de las propiedades de cada componente
            qDebug() << "Component Type:"
                     << QString::fromStdString(componentTypeToString(newComponent.getType()));
            for (const auto &prop : newComponent.getProps()) {
                qDebug() << "   Prop:" << QString::fromStdString(prop.first) << "="
                         << QString::fromStdString(prop.second);
            }

            // Debugging de componentes anidados
            if (newComponent.isAllowingItems()) {
                for (const auto &nestedComponent : newComponent.getNestedComponents()) {
                    qDebug() << "   Nested Component Type:"
                             << QString::fromStdString(
                                    componentTypeToString(nestedComponent.getType()));
                    for (const auto &nestedProp : nestedComponent.getProps()) {
                        qDebug() << "      Nested Prop:" << QString::fromStdString(nestedProp.first)
                                 << "=" << QString::fromStdString(nestedProp.second);
                    }
                }
            }

            newComponents.push_back(newComponent); // Conversión recursiva
        }

        // Reemplazar el contenido de currentView
        currentView.setComponents(newComponents);
        *it = currentView;
        qDebug() << "Updated currentView in views with new component structure.";
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

    // Usamos una pila de QTreeWidgetItem para guardar la jerarquía completa hasta el item actual
    std::vector<QTreeWidgetItem *> hierarchy;
    QTreeWidgetItem *currentItem = item;
    while (currentItem) {
        hierarchy.insert(hierarchy.begin(), currentItem);
        currentItem = currentItem->parent();
    }

    // Buscar el componente en el currentView usando la jerarquía de items
    Component *foundComponent = findComponentByHierarchy(currentView.getComponents(), hierarchy, 0);

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

        qDebug() << "Updated property" << propertyName << "to" << newValue;

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
    while (currentItem) {
        hierarchy.insert(hierarchy.begin(), currentItem);
        currentItem = currentItem->parent();
    }

    // Inicia la búsqueda en los componentes de primer nivel del view usando la jerarquía
    return findComponentByHierarchy(view.getComponents(), hierarchy, 0);
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
    // Agregar la ruta a la lista de rutas
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

    // Debug: Log de todos los componentes y sus nestedComponents
    // for (const auto &component : currentView.getComponents()) {
    //     qDebug() << "Component:"
    //              << QString::fromStdString(componentTypeToString(component.getType()));
    //     if (component.isAllowingItems()) {
    //         for (const auto &nestedComponent : component.getNestedComponents()) {
    //             qDebug() << "  Nested Component:"
    //                      << QString::fromStdString(componentTypeToString(nestedComponent.getType()));
    //         }
    //     }
    // }

    populateCurrentViewTree();
}
