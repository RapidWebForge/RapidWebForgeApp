#include "frontenddashboard.h"
#include <QDebug>
#include <QDropEvent>
#include <QFile>
#include <QMessageBox>
#include "../../models/component-type/componenttype.h"
#include "ui_frontenddashboard.h"
#include <assert.h>

FrontendDashboard::FrontendDashboard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FrontendDashboard)
    , createSectionDialog(nullptr)
{
    ui->setupUi(this);

    connect(ui->currentSectionTree,
            &CustomTreeWidget::itemDropped,
            this,
            &FrontendDashboard::onItemDropped);
    connect(ui->currentSectionTree,
            &QTreeWidget::itemClicked,
            this,
            &FrontendDashboard::onCurrentSectionTreeItemSelected);
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

    QFile primaryButtonstyleFile(":/styles/primarybutton");
    if (primaryButtonstyleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(primaryButtonstyleFile.readAll());
        ui->saveButton->setStyleSheet(styleSheet);
    }
    QFile secondaryButtonstyleFile(":/styles/secondarybutton");
    if (secondaryButtonstyleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(secondaryButtonstyleFile.readAll());
        ui->addSectionButton->setStyleSheet(styleSheet);
    }
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
    configureTreeWidget(ui->currentSectionTree, true, QAbstractItemView::DropOnly);
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
    std::vector<Component> &components = currentSection.getComponents();
    dropIndex = std::clamp(dropIndex, 0, static_cast<int>(components.size()));
    components.insert(components.begin() + dropIndex, newComponent);

    std::string componentType = componentTypeToString(newComponent.getType());
    QTreeWidgetItem *newItem = nullptr;

    qDebug() << componentType;

    if (componentType == "Custom") {
        auto it = newComponent.getProps().find("name");

        // for (const auto &prop : newComponent.getProps()) {
        //     qDebug() << "  " << QString::fromStdString(prop.first) << "="
        //              << QString::fromStdString(prop.second);
        // }

        if (it != newComponent.getProps().end()) {
            QString componentName = QString::fromStdString(it->second);
            newItem = createTreeItem(componentName);
        } else {
            QMessageBox::warning(this,
                                 "Custom Component Error",
                                 "Custom component name wasn't found.");
            return;
        }
    } else {
        newItem = createTreeItem(QString::fromStdString(componentType));
    }

    if (parentItem == ui->currentSectionTree->invisibleRootItem()) {
        ui->currentSectionTree->insertTopLevelItem(dropIndex, newItem);
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

// ComboBox Sections

void FrontendDashboard::fillAvailableSections()
{
    for (const auto &view : views) {
        ui->sectionComboBox->addItem(QString::fromStdString(view.getName()));
    }
    for (const auto &custComponent : custComponents) {
        ui->sectionComboBox->addItem(QString::fromStdString(custComponent.getName()));
    }
}

void FrontendDashboard::on_sectionComboBox_currentIndexChanged(int index)
{
    // Get the name of the selected section
    std::string newSectionSelected = ui->sectionComboBox->currentText().toStdString();

    // Find the section on views and custom components
    auto viewIt = std::find_if(views.begin(),
                               views.end(),
                               [&newSectionSelected](const Section &view) {
                                   return view.getName() == newSectionSelected;
                               });

    if (viewIt != views.end()) {
        setCurrentSection(*viewIt);
    } else {
        auto custComponentIt = std::find_if(custComponents.begin(),
                                            custComponents.end(),
                                            [&newSectionSelected](const Section &custComponent) {
                                                return custComponent.getName()
                                                       == newSectionSelected;
                                            });
        if (custComponentIt != custComponents.end()) {
            setCurrentSection(*custComponentIt);
        } else {
            QMessageBox::warning(this,
                                 "Section Error",
                                 "The view or custom comonent selected wasn't found.");
        }
    }
}

// CurrentSectionTree populate

void FrontendDashboard::populateCurrentSectionTree()
{
    ui->currentSectionTree->clear();

    // Add the currentSection as a first level item
    QTreeWidgetItem *viewItem = new QTreeWidgetItem(ui->currentSectionTree);
    viewItem->setText(0, QString::fromStdString(currentSection.getName()));

    // Add components of the current Section
    for (const auto &component : currentSection.getComponents()) {
        QTreeWidgetItem *componentItem = new QTreeWidgetItem(viewItem);
        componentItem->setText(0,
                               QString::fromStdString(componentTypeToString(component.getType())));

        // Verificar si el componente permite anidar y tiene subcomponentes
        if (component.isAllowingItems() && !component.getNestedComponents().empty()) {
            populateNestedItems(componentItem, component.getNestedComponents());
        }
    }

    ui->currentSectionTree->expandAll();
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
        Component *parentComponent = findComponentInTree(currentSection, parentItem);

        if (!parentComponent) {
            QMessageBox::warning(this, "Component error", "Parent wasn't found.");
            return;
        }

        insertNestedComponent(parentComponent, newComponent, parentItem, dropIndex);
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

void FrontendDashboard::onCurrentSectionTreeItemSelected(QTreeWidgetItem *item, int column)
{
    if (!item) {
        qDebug() << "Selected item is null.";
        return;
    }

    // Get the name of the selected item
    std::string selectedItemName = item->text(0).toStdString();

    // Check if it is a view
    auto viewIt = std::find_if(views.begin(), views.end(), [&selectedItemName](const Section &view) {
        return view.getName() == selectedItemName;
    });

    if (viewIt != views.end()) {
        // Update currentSection if it is a view
        currentSection = *viewIt;
        qDebug() << "Current section updated to the view:"
                 << QString::fromStdString(currentSection.getName());

        // ui->currentSectionLabel->setText(
        //     QString::fromStdString("Current View: " + currentSection.getName()));
        return;
    } else {
        // If not check in custom components
        auto custComponentIt = std::find_if(custComponents.begin(),
                                            custComponents.end(),
                                            [&selectedItemName](const Section &custComponent) {
                                                return custComponent.getName() == selectedItemName;
                                            });
        if (custComponentIt != custComponents.end()) {
            setCurrentSection(*custComponentIt);
            qDebug() << "Current section updated to the custom component:"
                     << QString::fromStdString(currentSection.getName());

            return;
        } else {
            // If is not a view or a custom component, check in the current section

            // Usamos una pila de QTreeWidgetItem para guardar la jerarquía completa hasta el item actual
            std::vector<QTreeWidgetItem *> hierarchy;
            QTreeWidgetItem *currentItem = item;
            while (currentItem) {
                hierarchy.insert(hierarchy.begin(), currentItem);
                currentItem = currentItem->parent();
            }

            // Buscar el componente en `currentSection` usando la jerarquía de items
            Component *foundComponent = findComponentByHierarchy(currentSection.getComponents(),
                                                                 hierarchy,
                                                                 1);

            if (foundComponent) {
                currentComponent = *foundComponent;
                qDebug() << "Found Component:"
                         << QString::fromStdString(
                                componentTypeToString(currentComponent.getType()));

                // Llenar la tabla de propiedades con las propiedades del componente
                populatePropertiesTable(currentComponent);
            } else {
                qDebug() << "Component of type"
                         << QString::fromStdString(item->text(0).toStdString()) << "not found.";
            }
        }
    }
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

        // Actualizar el componente en `currentSection` usando el método `findComponentInTree`
        Component *componentInSection = findComponentInTree(currentSection,
                                                            ui->currentSectionTree->currentItem());

        if (componentInSection) {
            componentInSection->setProps(currentComponent.getProps());

            qDebug() << "Property updated in currentSection. Updated Properties:";
            for (const auto &prop : componentInSection->getProps()) {
                qDebug() << "  " << QString::fromStdString(prop.first) << "="
                         << QString::fromStdString(prop.second);
            }
        } else {
            qDebug() << "Could not find component in currentSection.";
        }

        // Reflejar el currentSection en views or custom components
        auto viewIt = std::find_if(views.begin(), views.end(), [this](const Section &view) {
            return view.getName() == currentSection.getName();
        });

        if (viewIt != views.end()) {
            *viewIt = currentSection;
            qDebug() << "Current view updated in views.";
        } else {
            auto custComponentIt = std::find_if(custComponents.begin(),
                                                custComponents.end(),
                                                [this](const Section &custComponent) {
                                                    return custComponent.getName()
                                                           == currentSection.getName();
                                                });

            if (custComponentIt != custComponents.end()) {
                *custComponentIt = currentSection;
                qDebug() << "Current view updated in views.";
            } else {
                qDebug() << "Could not update current view in views.";
            }
        }
    }
}

Component *FrontendDashboard::findComponentInTree(Section &section, QTreeWidgetItem *item)
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
    if (hierarchy.size() > 0 && hierarchy[0]->text(0).toStdString() == section.getName()) {
        // Ignorar el nivel de la vista y comenzar desde el siguiente
        return findComponentByHierarchy(section.getComponents(), hierarchy, 1);
    } else {
        qDebug() << "Error: La vista no coincide con el nivel superior de hierarchy.";
        return nullptr;
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

// Component *FrontendDashboard::findNestedComponent(Component &parent, QTreeWidgetItem *item)
// {
//     const std::string type = item->text(0).toStdString();
//     std::vector<Component> &nestedComponents = parent.getNestedComponents();

//     qDebug() << "Searching for nested component of type" << QString::fromStdString(type);

//     for (auto &nestedComponent : nestedComponents) {
//         qDebug() << "Checking nested component of type"
//                  << QString::fromStdString(componentTypeToString(nestedComponent.getType()));

//         if (componentTypeToString(nestedComponent.getType()) == type && !item->parent()) {
//             qDebug() << "Nested component found.";
//             return &nestedComponent;
//         }

//         // Si el `QTreeWidgetItem` tiene más niveles, sigue buscando en niveles más profundos
//         if (nestedComponent.isAllowingItems() && item->parent()) {
//             Component *deepNested = findNestedComponent(nestedComponent, item->parent());
//             if (deepNested) {
//                 return deepNested;
//             }
//         }
//     }

//     qDebug() << "Nested component of type" << QString::fromStdString(type) << "not found.";
//     return nullptr;
// }

// New section saved from dialog

void FrontendDashboard::onRouteSaved(const Route &route)
{
    // Agregar la ruta a `routes`
    this->routes.push_back(route);

    // Crear una nueva vista y un componente `Header H1` predeterminado
    Section view(route.getComponent());
    Component headerComponent(ComponentType::HeaderH1);

    // Asignar propiedades predeterminadas al `Header H1`
    headerComponent.setProps({{"class", ""}, {"text", "Default Header"}});

    // Agregar `Header H1` a los componentes de la vista
    view.getComponents().push_back(headerComponent);
    view.setComponents(view.getComponents());

    // Sincronizar la vista con `currentSection` y `views`
    if (route.getComponent() == currentSection.getName()) {
        currentSection = view;
    }
    views.push_back(view);

    // Actualizar las rutas y vistas en la interfaz si es necesario
    setRoutes(routes);

    // Add the new view to combobox
    ui->sectionComboBox->addItem(QString::fromStdString(view.getName()));
}

void FrontendDashboard::onCustomComponentSaved(const Section &customComponent)
{
    this->custComponents.push_back(customComponent);

    // Get the first QTreeWidgetItem (Custom)
    QTreeWidgetItem *customComponents = ui->componentsTree->topLevelItem(0);

    // Crear un nuevo QTreeWidgetItem para el component y agregarlo al
    // `Custom` QTreeWidgetItem del `componentsTree`
    QTreeWidgetItem *newCustomComponent = new QTreeWidgetItem();

    newCustomComponent->setText(0, QString::fromStdString(customComponent.getName()));

    // Add the new component in case it have a name
    customComponents->addChild(newCustomComponent);

    setCustomComponents(this->custComponents);

    // Verification
    bool childAdded = (customComponents->childCount() > 0
                       && customComponents->child(customComponents->childCount() - 1)
                              == newCustomComponent);

    assert(childAdded && "Custom Component added");

    // Add the new view to combobox
    ui->sectionComboBox->addItem(QString::fromStdString(customComponent.getName()));
}

// Slots

void FrontendDashboard::on_saveButton_clicked()
{
    // Convert tree to section

    std::string sectionName = currentSection.getName();

    auto viewIt = std::find_if(views.begin(), views.end(), [&sectionName](const Section &s) {
        return s.getName() == sectionName;
    });

    if (viewIt != this->views.end()) {
        *viewIt = currentSection;
        QMessageBox::information(this, "Successful", "View updated");
        return;
    } else {
        auto custComponentIt = std::find_if(custComponents.begin(),
                                            custComponents.end(),
                                            [&sectionName](const Section &s) {
                                                return s.getName() == sectionName;
                                            });

        if (viewIt != this->custComponents.end()) {
            *viewIt = currentSection;
            QMessageBox::information(this, "Successful", "Custom Component updated");
            return;
        }
    }
    QMessageBox::warning(this, "Error on save", "View or Custom Component wasn't found");
}

void FrontendDashboard::on_deleteButton_clicked()
{
    // Get the selected item on `currentSectionTree`
    QTreeWidgetItem *selectedItem = ui->currentSectionTree->currentItem();
    if (!selectedItem) {
        QMessageBox::warning(this, "ERROR", "You haven't select an item");
        return;
    }

    // Get the name of the selected item
    std::string selectedItemName = selectedItem->text(0).toStdString();

    // Check if the item is a view
    auto viewIt = std::find_if(views.begin(), views.end(), [&selectedItemName](const Section &view) {
        return view.getName() == selectedItemName;
    });

    if (viewIt != views.end()) {
        // If is a view remove from `views` and `currentSectionTree`
        this->views.erase(viewIt);
        delete selectedItem; // Esto eliminará el elemento del árbol
        qDebug() << "View deleted:" << QString::fromStdString(selectedItemName);

        ui->sectionComboBox->removeItem(ui->sectionComboBox->currentIndex());
    } else {
        // If not check if is a custom component
        auto custComponentIt = std::find_if(custComponents.begin(),
                                            custComponents.end(),
                                            [&selectedItemName](const Section &custComponent) {
                                                return custComponent.getName() == selectedItemName;
                                            });

        if (custComponentIt != custComponents.end()) {
            // If is a view remove from `views` and `currentSectionTree`
            this->custComponents.erase(custComponentIt);
            delete selectedItem; // Esto eliminará el elemento del árbol
            qDebug() << "Custom component deleted:" << QString::fromStdString(selectedItemName);

            ui->sectionComboBox->removeItem(ui->sectionComboBox->currentIndex());
        } else {
            // Usamos una pila de QTreeWidgetItem para guardar la jerarquía completa hasta el item actual
            std::vector<QTreeWidgetItem *> hierarchy;
            QTreeWidgetItem *currentItem = selectedItem;
            while (currentItem) {
                hierarchy.insert(hierarchy.begin(), currentItem);
                currentItem = currentItem->parent();
            }

            // Buscar el componente en `currentSection` usando la jerarquía de items
            Component *foundComponent = findComponentByHierarchy(currentSection.getComponents(),
                                                                 hierarchy,
                                                                 1);

            if (foundComponent) {
                // Eliminar el componente de `currentSection`
                auto &components = currentSection.getComponents();
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
}

void FrontendDashboard::on_addSectionButton_clicked()
{
    // Show create section dialog
    if (!createSectionDialog) {
        createSectionDialog = new CreateSection(this);

        connect(createSectionDialog,
                &CreateSection::routeSaved,
                this,
                &FrontendDashboard::onRouteSaved);

        connect(createSectionDialog,
                &CreateSection::customComponentSaved,
                this,
                &FrontendDashboard::onCustomComponentSaved);
    }
    createSectionDialog->exec();
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

std::vector<Section> &FrontendDashboard::getViews()
{
    return views;
}

const std::vector<Section> &FrontendDashboard::getViews() const
{
    return views;
}

std::vector<Section> &FrontendDashboard::getCustomComponents()
{
    return custComponents;
}

const std::vector<Section> &FrontendDashboard::getCustomComponents() const
{
    return custComponents;
}

// Setters
void FrontendDashboard::setRoutes(const std::vector<Route> &routes)
{
    this->routes = routes;
}

void FrontendDashboard::setViews(const std::vector<Section> &views)
{
    this->views = views;
}

void FrontendDashboard::setCustomComponents(const std::vector<Section> &custComponents)
{
    this->custComponents = custComponents;
}

void FrontendDashboard::setCurrentSection(Section &section)
{
    currentSection = section;

    // ui->currentSectionLabel->setText(
    //     QString::fromStdString("Current View: " + currentSection.getName()));

    populateCurrentSectionTree();
}
