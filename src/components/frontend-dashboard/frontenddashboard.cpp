#include "frontenddashboard.h"
#include <QDebug>
#include <QDropEvent>
#include <QFile>
#include <QMessageBox>
#include "../../core/logging/actionloggerjson.h"
#include "../../models/component-type/componenttype.h"
#include "../../models/generic-node/genericnode.h"
#include "ui_frontenddashboard.h"
#include <boost/uuid/uuid_io.hpp>
#include <cassert>
#include <fmt/core.h>

FrontendDashboard::FrontendDashboard(QWidget *parent)
    : QWidget(parent)
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

void FrontendDashboard::addCustomComponentsOnComponentsTree()
{
    QList<QTreeWidgetItem *> items = ui->componentsTree->findItems("Custom",
                                                                   Qt::MatchExactly
                                                                       | Qt::MatchRecursive,
                                                                   0);

    if (items.isEmpty()) {
        qDebug() << "No se encontró el nodo 'Custom'.";
        return;
    }

    QTreeWidgetItem *customItem = items.first();

    auto customComponentsNode = getMainNode("CustomComponents");

    if (customComponentsNode) {
        auto sectionNode = std::dynamic_pointer_cast<GenericNode>(customComponentsNode);

        for (const auto &custComponent : sectionNode->getChildren()) {
            auto sectionPtr = std::dynamic_pointer_cast<Section>(custComponent);
            QString ccName = QString::fromStdString(sectionPtr->getName());
            createTreeItem(ccName, nullptr, customItem);
        }
    }
}

// ComboBox Sections

// For testing
// void printNodeTree(const std::shared_ptr<BaseNode> &node, int depth = 0)
// {
//     if (!node)
//         return;

//     QString indent = QString(" ").repeated(depth * 2);

//     if (auto component = std::dynamic_pointer_cast<Component>(node)) {
//         qDebug() << indent + "Component:";
//         qDebug().noquote() << indent + "  (ID: "
//                                   + QString::fromStdString(
//                                       boost::uuids::to_string(component->getId()))
//                                   + ")";
//         qDebug().noquote() << indent + "  (Type: "
//                                   + QString::fromStdString(
//                                       componentTypeToString(component->getType()))
//                                   + ")";

//     } else if (auto section = std::dynamic_pointer_cast<Section>(node)) {
//         qDebug() << indent + "Section:";
//         qDebug().noquote() << indent + "- " + QString::fromStdString(section->getName());
//         qDebug().noquote() << indent + "  (ID: "
//                                   + QString::fromStdString(boost::uuids::to_string(section->getId()))
//                                   + ")";
//     } else {
//         qDebug() << indent + "GenericNode or BaseNode:";
//         qDebug().noquote() << indent + "- " + QString::fromStdString(node->getNodeType());
//     }

//     // Recursively print children
//     for (const auto &child : node->getChildren()) {
//         printNodeTree(child, depth + 1);
//     }
// }

void FrontendDashboard::fillAvailableSections()
{
    // Limpia el combo box antes de rellenarlo
    ui->sectionComboBox->clear();

    // Itera sobre las vistas
    auto viewsNode = getMainNode("Views");

    if (viewsNode) {
        auto genericNode = std::dynamic_pointer_cast<GenericNode>(viewsNode);

        for (const auto &view : genericNode->getChildren()) {
            auto sectionPtr = std::dynamic_pointer_cast<Section>(view);
            if (sectionPtr) {
                ui->sectionComboBox->addItem(QString::fromStdString(sectionPtr->getName()));
                // qDebug() << "added " << QString::fromStdString(sectionPtr->getName());
            }
        }
    }

    // Itera sobre los custom components
    auto customComponentsNode = getMainNode("CustomComponents");

    if (customComponentsNode) {
        auto genericNode = std::dynamic_pointer_cast<GenericNode>(customComponentsNode);

        for (const auto &cc : genericNode->getChildren()) {
            auto sectionPtr = std::dynamic_pointer_cast<Section>(cc);
            if (sectionPtr) {
                ui->sectionComboBox->addItem(QString::fromStdString(sectionPtr->getName()));
            }
        }
    }
}

void FrontendDashboard::on_sectionComboBox_currentIndexChanged(int index)
{
    // Get the name of the selected section
    std::string newSectionSelected = ui->sectionComboBox->currentText().toStdString();

    // Search in views
    auto viewsNode = getMainNode("Views");

    if (!viewsNode) {
        qDebug() << "Views node not found on CURRENT INDEX CHANGED";
        return;
    }

    auto sectionNode = std::dynamic_pointer_cast<GenericNode>(viewsNode);

    if (sectionNode) {
        auto it = std::find_if(sectionNode->getChildren().begin(),
                               sectionNode->getChildren().end(),
                               [&newSectionSelected](const std::shared_ptr<BaseNode> &node) {
                                   auto section = std::dynamic_pointer_cast<Section>(node);
                                   return section && section->getName() == newSectionSelected;
                               });

        if (it != sectionNode->getChildren().end()) {
            setCurrentSection(*it);
            return;
        }
    }

    // Search in customcomponents

    auto customComponentsNode = getMainNode("CustomComponents");

    if (!customComponentsNode) {
        qDebug() << "Custom Components node not found on CURRENT INDEX CHANGED";
        return;
    }

    sectionNode = std::dynamic_pointer_cast<GenericNode>(customComponentsNode);

    if (sectionNode) {
        auto it = std::find_if(sectionNode->getChildren().begin(),
                               sectionNode->getChildren().end(),
                               [&newSectionSelected](const std::shared_ptr<BaseNode> &node) {
                                   auto section = std::dynamic_pointer_cast<Section>(node);
                                   return section && section->getName() == newSectionSelected;
                               });

        if (it != sectionNode->getChildren().end()) {
            setCurrentSection(*it);
            return;
        }
    }

    QMessageBox::warning(this,
                         "Section Error",
                         "The view or custom component selected wasn't found.");
}

// Start of populate current section tree

void FrontendDashboard::populateCurrentSectionTree()
{
    ui->currentSectionTree->clear();

    // printNodeTree(frontendRoot);

    // Verifica si currentSection es un Section
    auto sectionPtr = std::dynamic_pointer_cast<Section>(currentSection);
    if (!sectionPtr) {
        fmt::print(stderr, "Error: Current section is not a valid Section.\n");
        return;
    }

    // Agrega currentSection como un elemento de primer nivel
    QTreeWidgetItem *sectionItem = createTreeItem(QString::fromStdString(sectionPtr->getName()),
                                                  ui->currentSectionTree);

    // Agrega los componentes del Section
    for (const auto &child : sectionPtr->getChildren()) {
        if (auto componentPtr = std::dynamic_pointer_cast<Component>(child)) {
            // Si es un Component, crea un elemento del árbol
            QTreeWidgetItem *componentItem = createTreeItem(QString::fromStdString(
                                                                componentTypeToString(
                                                                    componentPtr->getType())),
                                                            nullptr,
                                                            sectionItem);
            componentItem->setData(0,
                                   Qt::UserRole,
                                   QString::fromStdString(
                                       boost::uuids::to_string(componentPtr->getId())));

            // Verifica si tiene subcomponentes
            if (componentPtr->isAllowingItems() && !componentPtr->getChildren().empty()) {
                populateNestedItems(componentItem, componentPtr->getChildren());
            }
        } else if (auto subSectionPtr = std::dynamic_pointer_cast<Section>(child)) {
            QTreeWidgetItem *subSectionItem = createTreeItem(QString::fromStdString(
                                                                 subSectionPtr->getName()),
                                                             nullptr,
                                                             sectionItem);
            subSectionItem->setData(0,
                                    Qt::UserRole,
                                    QString::fromStdString(
                                        boost::uuids::to_string(subSectionPtr->getId())));
        }
    }

    ui->currentSectionTree->expandAll();
}

void FrontendDashboard::populateNestedItems(
    QTreeWidgetItem *parentItem, const std::vector<std::shared_ptr<BaseNode>> &nestedComponents)
{
    for (const auto &nestedComponent : nestedComponents) {
        if (auto nestedComponentPtr = std::dynamic_pointer_cast<Component>(nestedComponent)) {
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
                && !nestedComponentPtr->getChildren().empty()) {
                populateNestedItems(nestedItem, nestedComponentPtr->getChildren());
            }
        } else if (auto subSectionPtr = std::dynamic_pointer_cast<Section>(nestedComponent)) {
            QTreeWidgetItem *nestedItem = createTreeItem(QString::fromStdString(
                                                             subSectionPtr->getName()),
                                                         nullptr,
                                                         parentItem);
            nestedItem->setData(0,
                                Qt::UserRole,
                                QString::fromStdString(
                                    boost::uuids::to_string(subSectionPtr->getId())));
        }
    }
}

// End of populate current section tree

// Auxiliar functions to onItemDropped

void FrontendDashboard::insertComponentInSection(std::shared_ptr<BaseNode> &newComponent,
                                                 QTreeWidgetItem *parentItem,
                                                 int dropIndex)
{
    // To insert in a custom component or a view is the same to insert in the cur
    auto sectionPtr = std::dynamic_pointer_cast<Section>(currentSection);
    if (!sectionPtr) {
        QMessageBox::warning(this, "Error", "Current section is not valid.");
        return;
    }

    const std::vector<std::shared_ptr<BaseNode>> &components = sectionPtr->getChildren();
    dropIndex = std::clamp(dropIndex, 0, static_cast<int>(components.size()));
    sectionPtr->insertChild(dropIndex, newComponent);

    if (auto componentPtr = std::dynamic_pointer_cast<Component>(newComponent)) {
        std::string componentType = componentTypeToString(componentPtr->getType());
        QTreeWidgetItem *newItem = createTreeItem(QString::fromStdString(componentType));
        newItem->setData(0,
                         Qt::UserRole,
                         QString::fromStdString(boost::uuids::to_string(componentPtr->getId())));
        parentItem->insertChild(dropIndex, newItem);
    } else if (auto newSectionPtr = std::dynamic_pointer_cast<Section>(newComponent)) {
        QTreeWidgetItem *newItem = createTreeItem(QString::fromStdString(newSectionPtr->getName()));
        newItem->setData(0,
                         Qt::UserRole,
                         QString::fromStdString(boost::uuids::to_string(newSectionPtr->getId())));
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

    dropIndex = std::clamp(dropIndex, 0, static_cast<int>(componentPtr->getChildren().size()));
    componentPtr->insertChild(dropIndex, newComponent);

    if (auto newComponentPtr = std::dynamic_pointer_cast<Component>(newComponent)) {
        QTreeWidgetItem *newItem = createTreeItem(
                QString::fromStdString(componentTypeToString(newComponentPtr->getType())));
        newItem->setData(0,
                         Qt::UserRole,
                         QString::fromStdString(boost::uuids::to_string(newComponentPtr->getId())));
        parentItem->insertChild(dropIndex, newItem);
    } else if (auto newSectionPtr = std::dynamic_pointer_cast<Section>(newComponent)) {
        QTreeWidgetItem *newItem = createTreeItem(QString::fromStdString(newSectionPtr->getName()));
        newItem->setData(0,
                         Qt::UserRole,
                         QString::fromStdString(boost::uuids::to_string(newSectionPtr->getId())));
        parentItem->insertChild(dropIndex, newItem);
    }
}

bool FrontendDashboard::isView(QTreeWidgetItem *item) const
{
    std::string itemType = item->text(0).toStdString();

    auto viewsNode = getMainNode("Views");

    if (!viewsNode) {
        qDebug() << "Views node not found on IS VIEW";
        return false;
    }

    auto sectionNode = std::dynamic_pointer_cast<GenericNode>(viewsNode);

    return std::any_of(sectionNode->getChildren().begin(),
                       sectionNode->getChildren().end(),
                       [&](const auto &v) {
                           auto section = std::dynamic_pointer_cast<Section>(v);
                           return section->getName() == itemType;
                       });
}

bool FrontendDashboard::isCustomComponent(QTreeWidgetItem *item) const
{
    std::string itemType = item->text(0).toStdString();

    auto customComponentsNode = getMainNode("CustomComponents");

    if (!customComponentsNode) {
        qDebug() << "Custom Components node not found on IS CUSTOM COMPONENT";
        return false;
    }

    auto sectionNode = std::dynamic_pointer_cast<GenericNode>(customComponentsNode);

    return std::any_of(sectionNode->getChildren().begin(),
                       sectionNode->getChildren().end(),
                       [&](const auto &cc) {
                           auto section = std::dynamic_pointer_cast<Section>(cc);
                           return section->getName() == itemType;
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

void FrontendDashboard::cleanPropertiesTable()
{
    // Limpiar la tabla de propiedades y ajustar el número de filas
    ui->propertiesTable->clearContents();
    ui->propertiesTable->setRowCount(0);
}

void FrontendDashboard::onCurrentSectionTreeItemSelected(QTreeWidgetItem *item, int column)
{
    if (!item) {
        qDebug() << "Selected item is null.";
        return;
    }

    qDebug() << "Finding:" << item->text(0);

    if (isView(item) || isCustomComponent(item)) {
        cleanPropertiesTable();
        return;
    }

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
    auto foundComponent = findComponentByHierarchy(currentSectionPtr->getChildren(),
                                                   hierarchy,
                                                   getComponentIdFromTree(item),
                                                   1);

    if (foundComponent) {
        currentComponent = foundComponent;

        auto currentComponentPtr = std::dynamic_pointer_cast<Component>(currentComponent);

        // qDebug() << "Found Component:"
        //          << QString::fromStdString(componentTypeToString(currentComponentPtr->getType()))
        //          << "with id" << QString::fromStdString(getComponentIdFromTree(item));

        // Llenar la tabla de propiedades con las propiedades del componente
        populatePropertiesTable(currentComponent);
    } else {
        cleanPropertiesTable();
        // qDebug() << "Component of type" << QString::fromStdString(item->text(0).toStdString())
        //          << "with id" << QString::fromStdString(getComponentIdFromTree(item))
        //          << "not found.";
    }
}

void FrontendDashboard::populatePropertiesTable(const std::shared_ptr<Component> &component)
{
    // Desconectar temporalmente onPropertyValueChanged para evitar bucles
    disconnect(ui->propertiesTable,
               &QTableWidget::cellChanged,
               this,
               &FrontendDashboard::onPropertyValueChanged);

    cleanPropertiesTable();

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
    }
}

std::shared_ptr<Component> FrontendDashboard::findComponentInTree(const std::shared_ptr<BaseNode> &section,
                                                                  QTreeWidgetItem *item)
{
    if (!section) {
        qDebug() << "Section is null.";
        return nullptr;
    }

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
        return findComponentByHierarchy(sectionPtr->getChildren(),
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
    const std::string &id,
    int level)
{
    if (level >= hierarchy.size())
        return nullptr; // Hemos recorrido toda la jerarquía sin encontrar el componente

    for (const auto &component : components) {
        auto componentPtr = std::dynamic_pointer_cast<Component>(component);

        if (!componentPtr) {
            continue; // Si no se puede convertir a Component, lo ignoramos
        }

        if (boost::uuids::to_string(componentPtr->getId()) == id
            && componentTypeToString(componentPtr->getType())
                   == hierarchy[level]->text(0).toStdString()) {
            // Si estamos en el último nivel de la jerarquía, devolvemos el componente encontrado
            if (level == hierarchy.size() - 1) {
                return componentPtr;
            }
        }

        // Si hay más niveles, continuamos buscando en los nestedComponents
        if (componentPtr->isAllowingItems()) {
            auto nestedComponent = findComponentByHierarchy(componentPtr->getChildren(),
                                                            hierarchy,
                                                            id,
                                                            level + 1);
            if (nestedComponent) {
                return nestedComponent;
            }
        }
    }

    return nullptr;
}

// New section saved from dialog

void FrontendDashboard::onSectionSaved(const std::shared_ptr<Section> &section)
{
    if (section->getPath().empty()) {
        // Custom Component
        auto customComponentsNode = getMainNode("CustomComponents");

        if (customComponentsNode) {
            auto sectionNode = std::dynamic_pointer_cast<GenericNode>(customComponentsNode);

            sectionNode->addChild(section);

            // Get the first QTreeWidgetItem (Custom)
            QTreeWidgetItem *customComponents = ui->componentsTree->topLevelItem(0);

            // Crear un nuevo QTreeWidgetItem para el component y agregarlo al
            // `Custom` QTreeWidgetItem del `componentsTree`
            QTreeWidgetItem *newCustomComponent = new QTreeWidgetItem();

            auto componentPtr = std::dynamic_pointer_cast<Section>(section);

            newCustomComponent->setText(0, QString::fromStdString(componentPtr->getName()));

            // Add the new component in case it have a name
            customComponents->addChild(newCustomComponent);

            // Verification
            bool childAdded = (customComponents->childCount() > 0
                               && customComponents->child(customComponents->childCount() - 1)
                                      == newCustomComponent);

            assert(childAdded && "Custom Component added");

            // Add the new view to combobox
            ui->sectionComboBox->addItem(QString::fromStdString(componentPtr->getName()));
        }

    } else {
        // Route
        auto viewsNode = getMainNode("Views");

        if (viewsNode) {
            auto sectionNode = std::dynamic_pointer_cast<GenericNode>(viewsNode);

            sectionNode->addChild(section);

            // Crear un componente `Header H1` predeterminado
            auto headerComponent = std::make_shared<Component>(ComponentType::HeaderH1);

            // Asignar propiedades predeterminadas al `Header H1`
            headerComponent->setProps({{"class", ""}, {"text", "Default Header"}});

            // Agregar `Header H1` a los componentes de la vista
            section->addChild(headerComponent);

            auto currentSectionPtr = std::dynamic_pointer_cast<Section>(currentSection);

            if (currentSectionPtr && section->getName() == currentSectionPtr->getName()) {
                setCurrentSection(section);
            }

            ui->sectionComboBox->addItem(QString::fromStdString(section->getName()));
        }
    }
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
    auto viewsNode = getMainNode("Views");

    if (!viewsNode) {
        qDebug() << "Views node is null.";
        return;
    }

    auto viewsPtr = std::dynamic_pointer_cast<GenericNode>(viewsNode);

    auto viewIt = std::find_if(viewsPtr->getChildren().begin(),
                               viewsPtr->getChildren().end(),
                               [&selectedItemName](const auto &node) {
                                   auto nodePtr = std::dynamic_pointer_cast<Section>(node);
                                   return nodePtr->getName() == selectedItemName;
                               });

    if (viewIt != viewsPtr->getChildren().end()) {
        viewsPtr->removeChild(viewIt);
        delete selectedItem;
        qDebug() << "View deleted:" << QString::fromStdString(selectedItemName);
        // Limpiar para evitar editar algo inexistente
        cleanPropertiesTable();

        ui->sectionComboBox->removeItem(ui->sectionComboBox->currentIndex());
        return;
    }

    // Busca en custom components
    auto customComponentsNode = getMainNode("CustomComponents");

    auto customComponentsPtr = std::dynamic_pointer_cast<GenericNode>(customComponentsNode);

    auto custComponentIt = std::find_if(customComponentsPtr->getChildren().begin(),
                                        customComponentsPtr->getChildren().end(),
                                        [&selectedItemName](const auto &node) {
                                            auto nodePtr = std::dynamic_pointer_cast<Section>(node);
                                            return nodePtr->getName() == selectedItemName;
                                        });

    // If the custom component is high level so remove, if not continue to search it as
    // a subcomponent or subsection
    if (custComponentIt != customComponentsPtr->getChildren().end() && !selectedItem->parent()) {
        customComponentsPtr->removeChild(custComponentIt);
        delete selectedItem;
        qDebug() << "Custom component deleted:" << QString::fromStdString(selectedItemName);
        // Limpiar para evitar editar algo inexistente
        cleanPropertiesTable();

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
        qDebug() << "Item deleted from section:" << QString::fromStdString(selectedItemName);
        // Limpiar para evitar editar algo inexistente
        cleanPropertiesTable();
    } else {
        qDebug() << "Failed to delete item: Not found in hierarchy.";
    }
}

bool FrontendDashboard::deleteComponentByHierarchy(const std::shared_ptr<BaseNode> &parent,
                                                   const std::vector<QTreeWidgetItem *> &hierarchy)
{
    if (hierarchy.empty()) {
        return false;
    }

    const std::string &targetId = getComponentIdFromTree(hierarchy.back());

    // Usamos el método encapsulado para eliminar el componente
    auto parentSection = std::dynamic_pointer_cast<Section>(parent);
    if (parentSection && parentSection->removeChildForById(targetId))
        return true;

    auto parentComponent = std::dynamic_pointer_cast<Component>(parent);
    if (parentComponent && parentComponent->removeChildForById(targetId))
        return true;

    // Si no está en los componentes directos, buscar en subcomponentes
    auto &components = parent->getChildren();
    for (const auto &child : components) {
        // auto  = std::dynamic_pointer_cast<BaseNode>(child);
        if (child && deleteComponentByHierarchy(child, hierarchy)) {
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
                &CreateSection::onSectionSaved,
                this,
                &FrontendDashboard::onSectionSaved);
    }
    createSectionDialog->exec();
}

// Getters
const std::shared_ptr<BaseNode> FrontendDashboard::getMainNode(const std::string &nodeName) const
{
    if (!frontendRoot) {
        qDebug() << "frontendRoot is null. Please initialize it before accessing main nodes.";
        return nullptr;
    }
    auto it = std::find_if(frontendRoot->getChildren().begin(),
                           frontendRoot->getChildren().end(),
                           [&nodeName](const auto &node) {
                               return node->getNodeType() == nodeName;
                           });

    if (it != frontendRoot->getChildren().end()) {
        return *it; // Devuelve el nodo encontrado
    }

    qDebug() << "The node" << QString::fromStdString(nodeName) << "wasn't found";
    return nullptr; // No se encontró el nodo
}

// Setters

void FrontendDashboard::setFrontendRoot(const std::shared_ptr<BaseNode> &frontendRoot)
{
    this->frontendRoot = frontendRoot;
}

void FrontendDashboard::setCurrentSection(const std::shared_ptr<BaseNode> &section)
{
    if (!section) {
        qDebug() << "Attempted to set current section to null.";
        return;
    }
    auto sectionPtr = std::dynamic_pointer_cast<Section>(section);
    if (!sectionPtr) {
        qDebug() << "Provided node is not a valid Section.";
        return;
    }
    this->currentSection = sectionPtr;
    populateCurrentSectionTree();
}
