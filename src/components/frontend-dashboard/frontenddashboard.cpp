#include "frontenddashboard.h"
#include <QDebug>
#include <QDropEvent>
#include <QFile>
#include <QMessageBox>
#include "../../core/logging/actionloggerjson.h"
#include "../../models/component-type/componenttype.h"
#include "ui_frontenddashboard.h"
#include <boost/uuid/uuid_io.hpp>
#include <cassert>
#include <fmt/core.h>

FrontendDashboard::FrontendDashboard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FrontendDashboard)
    , createSectionDialog(nullptr)
    , loggerJson("resources/logs/user_actions.json") // Cambiar la ruta al archivo JSON

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

// Start of general auxiliar functions

QTreeWidgetItem *FrontendDashboard::createTreeItem(const QString &text,
                                                   CustomTreeWidget *treeWidget,
                                                   QTreeWidgetItem *parentItem)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();

    if (treeWidget) {
        // Si el parent es un QTreeWidget, agregar el item como nivel superior
        treeWidget->addTopLevelItem(item);
    } else if (parentItem) {
        // Si el parent es un QTreeWidgetItem, agregarlo como hijo
        parentItem->addChild(item);
    }

    item->setText(0, text);
    return item;
}

std::string FrontendDashboard::getComponentIdFromTree(QTreeWidgetItem *item) const
{
    // Retrieve the ID from the item using Qt::UserRole
    QVariant idData = item->data(0, Qt::UserRole);
    if (idData.isValid()) {
        return idData.toString().toStdString();
    }

    return {};
}

// End of general auxiliar functions

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

// ComboBox Sections

void FrontendDashboard::fillAvailableSections()
{
    // Limpia el combo box antes de rellenarlo
    ui->sectionComboBox->clear();

    // Itera sobre las vistas
    for (const auto &view : views) {
        auto sectionPtr = std::dynamic_pointer_cast<Section>(view);
        if (sectionPtr) {
            ui->sectionComboBox->addItem(QString::fromStdString(sectionPtr->getName()));
        }
    }

    // Itera sobre los custom components
    for (const auto &custComponent : custComponents) {
        auto sectionPtr = std::dynamic_pointer_cast<Section>(custComponent);
        if (sectionPtr) {
            ui->sectionComboBox->addItem(QString::fromStdString(sectionPtr->getName()));
        }
    }
}

void FrontendDashboard::on_sectionComboBox_currentIndexChanged(int index)
{
    // Get the name of the selected section
    std::string newSectionSelected = ui->sectionComboBox->currentText().toStdString();

    // Busca la sección en views
    auto viewIt = std::find_if(views.begin(),
                               views.end(),
                               [&newSectionSelected](const std::shared_ptr<Section> &node) {
                                   return node->getName() == newSectionSelected;
                               });

    if (viewIt != views.end()) {
        setCurrentSection(*viewIt);
        return;
    }

    // Busca la sección en custom components
    auto custComponentIt = std::find_if(custComponents.begin(),
                                        custComponents.end(),
                                        [&newSectionSelected](const std::shared_ptr<Section> &node) {
                                            return node->getName() == newSectionSelected;
                                        });

    if (custComponentIt != custComponents.end()) {
        setCurrentSection(*custComponentIt);
    } else {
        QMessageBox::warning(this,
                             "Section Error",
                             "The view or custom component selected wasn't found.");
    }
}

// Start of populate current section tree

void FrontendDashboard::populateCurrentSectionTree()
{
    ui->currentSectionTree->clear();

    // Verifica si currentSection es un Section
    auto sectionPtr = std::dynamic_pointer_cast<Section>(currentSection);
    if (!sectionPtr) {
        fmt::print(stderr, "Error: Current section is not a valid Section.\n");
        return;
    }

    // Agrega currentSection como un elemento de primer nivel
    QTreeWidgetItem *viewItem = createTreeItem(QString::fromStdString(sectionPtr->getName()), ui->currentSectionTree);

    // Agrega los componentes del Section
    for (const auto &child : sectionPtr->getComponents()) {
        auto componentPtr = std::dynamic_pointer_cast<Component>(child);
        if (componentPtr) {
            // Si es un Component, crea un elemento del árbol
            QTreeWidgetItem *componentItem = createTreeItem(QString::fromStdString(
                                                                componentTypeToString(
                                                                    componentPtr->getType())),
                                                            nullptr,
                                                            viewItem);
            componentItem->setData(0,
                                   Qt::UserRole,
                                   QString::fromStdString(
                                       boost::uuids::to_string(componentPtr->getId())));

            // Verifica si tiene subcomponentes
            if (componentPtr->isAllowingItems() && !componentPtr->getNestedComponents().empty()) {
                populateNestedItems(componentItem, componentPtr->getNestedComponents());
            }
        } else {
            auto subSectionPtr = std::dynamic_pointer_cast<Section>(child);
            if (subSectionPtr) {
                // Si es un Section, llama recursivamente para agregar sus componentes
                populateSubSectionItems(viewItem, subSectionPtr);
            }
        }
    }

    ui->currentSectionTree->expandAll();
}

void FrontendDashboard::populateNestedItems(
    QTreeWidgetItem *parentItem, const std::vector<std::shared_ptr<BaseNode>> &nestedComponents)
{
    for (const auto &nestedComponent : nestedComponents) {
        auto nestedComponentPtr = std::dynamic_pointer_cast<Component>(nestedComponent);

        if (nestedComponentPtr) {
            QTreeWidgetItem *nestedItem = createTreeItem(QString::fromStdString(
                                                             componentTypeToString(
                                                                 nestedComponentPtr->getType())),
                                                         nullptr,
                                                         parentItem);
            nestedItem->setData(0,
                                Qt::UserRole,
                                QString::fromStdString(
                                    boost::uuids::to_string(nestedComponentPtr->getId())));

            if (nestedComponentPtr->isAllowingItems()
                && !nestedComponentPtr->getNestedComponents().empty()) {
                populateNestedItems(nestedItem, nestedComponentPtr->getNestedComponents());
            }
        } else {
            auto subSectionPtr = std::dynamic_pointer_cast<Section>(nestedComponent);
            if (subSectionPtr) {
                // Si es un Section, llama recursivamente para agregar sus componentes
                populateSubSectionItems(parentItem, subSectionPtr);
            }
        }
    }
}

void FrontendDashboard::populateSubSectionItems(QTreeWidgetItem *parentItem,
                                                const std::shared_ptr<Section> &subSection)
{
    QTreeWidgetItem *subSectionItem = new QTreeWidgetItem(parentItem);
    subSectionItem->setText(0, QString::fromStdString(subSection->getName()));
}

// End of populate current section tree

// Auxiliar functions to onItemDropped

void FrontendDashboard::insertComponentInSection(std::shared_ptr<BaseNode> &newComponent,
                                                 QTreeWidgetItem *parentItem,
                                                 int dropIndex)
{
    auto sectionPtr = std::dynamic_pointer_cast<Section>(currentSection);
    if (!sectionPtr) {
        QMessageBox::warning(this, "Error", "Current section is not valid.");
        return;
    }

    const std::vector<std::shared_ptr<BaseNode>> &components = sectionPtr->getComponents();
    dropIndex = std::clamp(dropIndex, 0, static_cast<int>(components.size()));
    sectionPtr->insertComponent(dropIndex, newComponent);

    auto componentPtr = std::dynamic_pointer_cast<Component>(newComponent);

    std::string componentType = componentTypeToString(componentPtr->getType());
    QTreeWidgetItem *newItem = nullptr;

    // qDebug() << QString::fromStdString(componentType);

    newItem = createTreeItem(QString::fromStdString(componentType));
    newItem->setData(0,
                     Qt::UserRole,
                     QString::fromStdString(boost::uuids::to_string(componentPtr->getId())));

    if (parentItem == ui->currentSectionTree->invisibleRootItem()) {
        ui->currentSectionTree->insertTopLevelItem(dropIndex, newItem);
    } else {
        parentItem->insertChild(dropIndex, newItem);
    }
}

void FrontendDashboard::insertNestedComponent(std::shared_ptr<Component> &parentComponent,
                                              std::shared_ptr<BaseNode> &newComponent,
                                              QTreeWidgetItem *parentItem,
                                              int dropIndex)
{
    auto componentPtr = std::dynamic_pointer_cast<Component>(parentComponent);
    if (!componentPtr || !componentPtr->isAllowingItems()) {
        QMessageBox::warning(this, "Invalid Operation", "This component does not allow nesting.");
        return;
    }

    auto nestedComponents = componentPtr->getNestedComponents();

    dropIndex = std::clamp(dropIndex, 0, static_cast<int>(nestedComponents.size()));

    nestedComponents.insert(nestedComponents.begin() + dropIndex, newComponent);

    auto newComponentPtr = std::dynamic_pointer_cast<Component>(newComponent);

    if (newComponent) {
        QTreeWidgetItem *newItem = createTreeItem(
                QString::fromStdString(componentTypeToString(newComponentPtr->getType())));
        newItem->setData(0,
                         Qt::UserRole,
                         QString::fromStdString(boost::uuids::to_string(newComponentPtr->getId())));
        parentItem->insertChild(dropIndex, newItem);
    } else {
        auto newSectionPtr = std::dynamic_pointer_cast<Section>(newComponent);
        QTreeWidgetItem *newItem = createTreeItem(QString::fromStdString(newSectionPtr->getName()));
        parentItem->insertChild(dropIndex, newItem);
    }
}

bool FrontendDashboard::isView(QTreeWidgetItem *item) const
{
    std::string itemType = item->text(0).toStdString();
    return std::any_of(views.begin(), views.end(), [&](const auto &v) {
        return v->getName() == itemType;
    });
}

bool FrontendDashboard::isCustomComponent(QTreeWidgetItem *item) const
{
    std::string itemType = item->text(0).toStdString();
    return std::any_of(custComponents.begin(), custComponents.end(), [&](const auto &cc) {
        return cc->getName() == itemType;
    });
}

std::shared_ptr<BaseNode> FrontendDashboard::convertItemToBaseNode(QTreeWidgetItem *item)
{
    if (!item) {
        return nullptr;
    }

    QString itemText = item->text(0);

    if (isView(item) || isCustomComponent(item)) {
        auto section = std::make_shared<Section>(itemText.toStdString());
        return section;
    } else {
        auto component = std::make_shared<Component>(stringToComponentType(itemText.toStdString()));
        return component;
    }
}

// End of Auxiliar functions to onItemDropped

void FrontendDashboard::onItemDropped(QTreeWidgetItem *parentItem,
                                      QTreeWidgetItem *droppedItem,
                                      int dropIndex) {
    if (!parentItem || !droppedItem) {
        qDebug() << "Error: null items in onItemDropped";
        return;
    }

    auto newNode = convertItemToBaseNode(droppedItem);

    if (!newNode) {
        QMessageBox::warning(this, "Component Error", "Failed to convert dropped item.");
        return;
    }

    if (isView(parentItem) || isCustomComponent(parentItem)) {
        insertComponentInSection(newNode, parentItem, dropIndex);
    } else {
        auto parentComponent = findComponentInTree(currentSection, parentItem);

        if (!parentComponent) {
            QMessageBox::warning(this, "Component Error", "Parent wasn't found.");
            return;
        }

        insertNestedComponent(parentComponent, newNode, parentItem, dropIndex);
    }
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
    auto viewIt = std::find_if(views.begin(),
                               views.end(),
                               [&selectedItemName](const std::shared_ptr<Section> &view) {
                                   return view->getName() == selectedItemName;
                               });

    if (viewIt != views.end()) {
        // Update currentSection if it is a view
        setCurrentSection(*viewIt);
        auto viewPtr = std::dynamic_pointer_cast<Section>(*viewIt);

        qDebug() << "Current section updated to the view:"
                 << QString::fromStdString(viewPtr->getName());
    } else {
        // If not check in custom components
        auto custComponentIt = std::find_if(custComponents.begin(),
                                            custComponents.end(),
                                            [&selectedItemName](
                                                const std::shared_ptr<Section> &custComponent) {
                                                return custComponent->getName() == selectedItemName;
                                            });

        if (custComponentIt != custComponents.end()) {
            setCurrentSection(*custComponentIt);
            auto custComponentPtr = std::dynamic_pointer_cast<Section>(*custComponentIt);

            qDebug() << "Current section updated to the custom component:"
                     << QString::fromStdString(custComponentPtr->getName());
        } else {
            // If is not a view or a custom component, check in the current section

            // Usamos una pila de QTreeWidgetItem para guardar la jerarquía completa hasta el item actual
            std::vector<QTreeWidgetItem *> hierarchy;
            QTreeWidgetItem *currentItem = item;
            while (currentItem) {
                hierarchy.insert(hierarchy.begin(), currentItem);
                currentItem = currentItem->parent();
            }

            auto currentSectionPtr = std::dynamic_pointer_cast<Section>(currentSection);

            // Buscar el componente en `currentSection` usando la jerarquía de items
            std::shared_ptr<Component> foundComponent
                = findComponentByHierarchy(currentSectionPtr->getComponents(),
                                           hierarchy,
                                           getComponentIdFromTree(item),
                                           1);

            if (foundComponent) {
                currentComponent = foundComponent;

                auto currentComponentPtr = std::dynamic_pointer_cast<Component>(currentComponent);

                qDebug() << "Found Component:"
                         << QString::fromStdString(
                                componentTypeToString(currentComponentPtr->getType()));

                // Llenar la tabla de propiedades con las propiedades del componente
                populatePropertiesTable(currentComponent);
            } else {
                qDebug() << "Component of type"
                         << QString::fromStdString(item->text(0).toStdString()) << "not found.";
            }
        }
    }
}

void FrontendDashboard::populatePropertiesTable(const std::shared_ptr<Component> &component)
{
    // Desconectar temporalmente onPropertyValueChanged para evitar bucles
    disconnect(ui->propertiesTable,
               &QTableWidget::cellChanged,
               this,
               &FrontendDashboard::onPropertyValueChanged);

    // Limpiar la tabla de propiedades y ajustar el número de filas
    ui->propertiesTable->clearContents();
    ui->propertiesTable->setRowCount(0);

    auto componentPtr = std::dynamic_pointer_cast<Component>(component);

    const auto &props = componentPtr->getProps();

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
        auto componentPtr = std::dynamic_pointer_cast<Component>(currentComponent);
        std::map<std::string, std::string> currentProps = componentPtr->getProps();
        currentProps[propertyName.toStdString()] = newValue.toStdString();
        componentPtr->setProps(currentProps);
        // Registrar log de la acción
        std::string logMessage = "Property '" + propertyName.toStdString() + "' updated to '"
                                 + newValue.toStdString() + "'";
        loggerJson.logAction("edit-tag-attributes", logMessage);

        // --- Nueva lógica para detectar estilos de Tailwind ---
        if (propertyName == "class") {
            std::string classValue = newValue.toStdString();

            // Lista de logs y patrones de Tailwind a detectar
            std::vector<std::pair<std::string, std::string>> tailwindLogs = {
                {"apply-text-styling", "text-"}, // Texto rojo, tamaño, cursiva, etc.
                {"use-flexbox-grid", "flex"},    // Flexbox
                {"use-flexbox-grid", "grid"},    // Grid Layout
                {"responsive-design", "sm:"},    // Diseño responsive
                {"responsive-design", "md:"},
                {"responsive-design", "lg:"},
                {"responsive-design", "xl:"},
                {"responsive-design", "2xl:"},
                {"responsive-design", "bg-"} // Cambio de color de fondo
            };

            for (const auto &[logType, pattern] : tailwindLogs) {
                if (classValue.find(pattern) != std::string::npos) {
                    loggerJson.logAction(logType,
                                         "User applied '" + pattern + "' in class property.");
                    break; // Para evitar múltiples registros del mismo cambio
                }
            }
        }

        // Actualizar el componente en `currentSection` usando el método `findComponentInTree`
        std::shared_ptr<Component> componentInSection = findComponentInTree(currentSection,
                                                            ui->currentSectionTree->currentItem());

        if (componentInSection) {
            auto componentPtr = std::dynamic_pointer_cast<Component>(componentInSection);
            componentInSection->setProps(componentPtr->getProps());

            qDebug() << "Property updated in currentSection. Updated Properties:";
            for (const auto &prop : componentInSection->getProps()) {
                qDebug() << "  " << QString::fromStdString(prop.first) << "="
                         << QString::fromStdString(prop.second);
            }
        } else {
            qDebug() << "Could not find component in currentSection.";
        }

        // Reflejar el currentSection en views or custom components
        auto viewIt = std::find_if(views.begin(),
                                   views.end(),
                                   [this](const std::shared_ptr<Section> &view) {
                                       return view->getName() == currentSection->getName();
                                   });

        if (viewIt != views.end()) {
            *viewIt = currentSection;
            qDebug() << "Current view updated in views.";
        } else {
            auto custComponentIt = std::find_if(custComponents.begin(),
                                                custComponents.end(),
                                                [this](
                                                    const std::shared_ptr<Section> &custComponent) {
                                                    return custComponent->getName()
                                                           == currentSection->getName();
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

std::shared_ptr<Component> FrontendDashboard::findComponentInTree(const std::shared_ptr<BaseNode> &section,
                                                                  QTreeWidgetItem *item)
{
    // Usamos la jerarquía completa de item para realizar una búsqueda exacta
    std::vector<QTreeWidgetItem *> hierarchy;
    QTreeWidgetItem *currentItem = item;

    // Construir la jerarquía desde el item hasta la raíz
    while (currentItem) {
        hierarchy.insert(hierarchy.begin(), currentItem);
        currentItem = currentItem->parent();
    }

    auto sectionPtr = std::dynamic_pointer_cast<Section>(section);
    if (!sectionPtr) {
        qDebug() << "Error: currentSection no es una Section.";
        return nullptr;
    }

    // Comprobamos que el primer nivel en hierarchy es la seccion
    if (!hierarchy.empty() && hierarchy[0]->text(0).toStdString() == sectionPtr->getName()) {
        // Ignorar el nivel de la vista y comenzar desde el siguiente
        return findComponentByHierarchy(sectionPtr->getComponents(),
                                        hierarchy,
                                        getComponentIdFromTree(item),
                                        1);
    }

    qDebug() << "Error: La vista no coincide con el nivel superior de hierarchy.";
    return nullptr;
}

// Función recursiva que busca el componente utilizando la jerarquía
std::shared_ptr<Component> FrontendDashboard::findComponentByHierarchy(
    const std::vector<std::shared_ptr<BaseNode>> &components,
    const std::vector<QTreeWidgetItem *> &hierarchy,
    std::string id,
    int level)
{
    if (level >= hierarchy.size()) {
        return nullptr;
    }

    const std::string type = hierarchy[level]->text(0).toStdString();

    for (auto &component : components) {
        auto componentPtr = std::dynamic_pointer_cast<Component>(component);

        if (componentPtr && componentTypeToString(componentPtr->getType()) == type
            && boost::uuids::to_string(componentPtr->getId()) == id) {
            // Si estamos en el último nivel de la jerarquía, devolvemos el componente encontrado
            if (level == hierarchy.size() - 1) {
                return componentPtr;
            }

            // Si hay más niveles, continuamos buscando en los nestedComponents
            if (componentPtr->isAllowingItems()) {
                auto nested = findComponentByHierarchy(componentPtr->getNestedComponents(),
                                                       hierarchy,
                                                       id,
                                                       level + 1);
                if (nested) {
                    return nested;
                }
            }
        }
    }

    return nullptr;
}

// New section saved from dialog

void FrontendDashboard::onRouteSaved(const Route &route)
{
    // Agregar la ruta a `routes`
    this->routes.push_back(route);

    // Crear una nueva vista como `Section`
    auto view = std::make_shared<Section>(route.getComponent());
    // Crear un componente `Header H1` predeterminado
    auto headerComponent = std::make_shared<Component>(ComponentType::HeaderH1);

    // Asignar propiedades predeterminadas al `Header H1`
    headerComponent->setProps({{"class", ""}, {"text", "Default Header"}});

    // Agregar `Header H1` a los componentes de la vista
    view->addComponent(headerComponent);

    auto sectionPtr = std::dynamic_pointer_cast<Section>(currentSection);

    // Verificar si la nueva vista coincide con `currentSection`
    auto currentSectionPtr = std::dynamic_pointer_cast<Section>(currentSection);
    if (currentSectionPtr && route.getComponent() == currentSectionPtr->getName()) {
        setCurrentSection(view);
    }

    // Agregar la nueva vista a `views`
    views.push_back(view);

    // Actualizar las rutas y vistas en la interfaz si es necesario
    setRoutes(routes);

    // Add the new view to combobox
    ui->sectionComboBox->addItem(QString::fromStdString(view->getName()));
}

void FrontendDashboard::onCustomComponentSaved(const std::shared_ptr<Section> &customComponent)
{
    this->custComponents.push_back(customComponent);

    // Get the first QTreeWidgetItem (Custom)
    QTreeWidgetItem *customComponents = ui->componentsTree->topLevelItem(0);

    // Crear un nuevo QTreeWidgetItem para el component y agregarlo al
    // `Custom` QTreeWidgetItem del `componentsTree`
    QTreeWidgetItem *newCustomComponent = new QTreeWidgetItem();

    auto componentPtr = std::dynamic_pointer_cast<Section>(customComponent);

    newCustomComponent->setText(0, QString::fromStdString(componentPtr->getName()));

    // Add the new component in case it have a name
    customComponents->addChild(newCustomComponent);

    setCustomComponents(this->custComponents);

    // Verification
    bool childAdded = (customComponents->childCount() > 0
                       && customComponents->child(customComponents->childCount() - 1)
                              == newCustomComponent);

    assert(childAdded && "Custom Component added");

    // Add the new view to combobox
    ui->sectionComboBox->addItem(QString::fromStdString(componentPtr->getName()));
}

// Slots

void FrontendDashboard::on_saveButton_clicked()
{
    if (!currentSection) {
        QMessageBox::warning(this, "Error on save", "No current section selected");
        return;
    }

    // Convertir tree a Section y obtener el nombre
    auto sectionPtr = std::dynamic_pointer_cast<Section>(currentSection);
    if (!sectionPtr) {
        QMessageBox::warning(this, "Error on save", "Current section is not valid");
        return;
    }
    std::string sectionName = sectionPtr->getName();

    // Buscar en views
    auto viewIt = std::find_if(views.begin(),
                               views.end(),
                               [&sectionName](const std::shared_ptr<Section> &node) {
                                   return node->getName() == sectionName;
                               });

    if (viewIt != views.end()) {
        *viewIt = sectionPtr;
        QMessageBox::information(this, "Successful", "View updated");
        return;
    }

    // Buscar en custom components
    auto custComponentIt = std::find_if(custComponents.begin(),
                                        custComponents.end(),
                                        [&sectionName](const std::shared_ptr<Section> &node) {
                                            return node->getName() == sectionName;
                                        });

    if (custComponentIt != custComponents.end()) {
        *custComponentIt = sectionPtr;
        QMessageBox::information(this, "Successful", "Custom Component updated");
        return;
    }

    QMessageBox::warning(this, "Error on save", "View or Custom Component wasn't found");
}

void FrontendDashboard::on_deleteButton_clicked()
{
    // Obtiene el elemento seleccionado en el árbol
    QTreeWidgetItem *selectedItem = ui->currentSectionTree->currentItem();
    if (!selectedItem) {
        QMessageBox::warning(this, "ERROR", "You haven't selected an item");
        return;
    }

    // Obtiene el nombre del elemento seleccionado
    std::string selectedItemName = selectedItem->text(0).toStdString();

    // Busca en views
    auto viewIt = std::find_if(views.begin(),
                               views.end(),
                               [&selectedItemName](const std::shared_ptr<Section> &node) {
                                   return node->getName() == selectedItemName;
                               });

    if (viewIt != views.end()) {
        views.erase(viewIt);
        delete selectedItem;
        qDebug() << "View deleted:" << QString::fromStdString(selectedItemName);

        ui->sectionComboBox->removeItem(ui->sectionComboBox->currentIndex());
        return;
    }

    // Busca en custom components
    auto custComponentIt = std::find_if(custComponents.begin(),
                                        custComponents.end(),
                                        [&selectedItemName](const std::shared_ptr<Section> &node) {
                                            return node->getName() == selectedItemName;
                                        });

    if (custComponentIt != custComponents.end()) {
        custComponents.erase(custComponentIt);
        delete selectedItem;
        qDebug() << "Custom component deleted:" << QString::fromStdString(selectedItemName);

        ui->sectionComboBox->removeItem(ui->sectionComboBox->currentIndex());
        return;
    }

    // Verifica si currentSection es un Section
    auto currentSectionPtr = std::dynamic_pointer_cast<Section>(currentSection);
    if (!currentSectionPtr) {
        QMessageBox::warning(this, "Error", "Current section is not valid for deletion");
        return;
    }

    // Busca el componente dentro de la jerarquía del árbol
    std::vector<QTreeWidgetItem *> hierarchy;
    QTreeWidgetItem *currentItem = selectedItem;
    while (currentItem) {
        hierarchy.insert(hierarchy.begin(), currentItem);
        currentItem = currentItem->parent();
    }

    // Llama a una función para eliminar un componente basado en la jerarquía
    if (deleteComponentByHierarchy(currentSectionPtr, hierarchy)) {
        delete selectedItem;
        qDebug() << "Component deleted from section:" << QString::fromStdString(selectedItemName);
    } else {
        qDebug() << "Failed to delete component: Not found in hierarchy.";
    }
}

bool FrontendDashboard::deleteComponentByHierarchy(const std::shared_ptr<Section> &section,
                                                   const std::vector<QTreeWidgetItem *> &hierarchy)
{
    if (hierarchy.empty()) {
        return false;
    }

    const std::string &targetName = hierarchy.back()->text(0).toStdString();

    // Usamos el método encapsulado en Section para eliminar el componente
    if (section->removeComponentByName(targetName)) {
        return true;
    }

    // Si no está en los componentes directos, buscar en subsecciones
    auto &components = section->getComponents();
    for (const auto &child : components) {
        auto subSection = std::dynamic_pointer_cast<Section>(child);
        if (subSection && deleteComponentByHierarchy(subSection, hierarchy)) {
            return true;
        }
    }

    return false;
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

const std::vector<std::shared_ptr<Section>> &FrontendDashboard::getViews() const
{
    return views;
}

const std::vector<std::shared_ptr<Section>> &FrontendDashboard::getCustomComponents() const
{
    return custComponents;
}

// Setters
void FrontendDashboard::setRoutes(const std::vector<Route> &routes)
{
    this->routes = routes;
}

void FrontendDashboard::setViews(const std::vector<std::shared_ptr<Section>> &views)
{
    this->views = views;
}

void FrontendDashboard::setCustomComponents(
    const std::vector<std::shared_ptr<Section>> &custComponents)
{
    this->custComponents = custComponents;
}

void FrontendDashboard::setCurrentSection(const std::shared_ptr<Section> &section)
{
    this->currentSection = section;

    populateCurrentSectionTree();
}
