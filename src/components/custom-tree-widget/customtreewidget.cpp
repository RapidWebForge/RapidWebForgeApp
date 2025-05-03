// customtreewidget.cpp
#include "customtreewidget.h"
#include <QApplication>
#include <QDebug>
#include <QDrag>
#include <QDropEvent>
#include <QMimeData>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QTreeWidget>
#include "../../models/component-type/componenttype.h"
#include "../../models/component/component.h"

CustomTreeWidget::CustomTreeWidget(QWidget *parent)
    : QTreeWidget(parent)

{
    setDragEnabled(true);
    setAcceptDrops(true);
    // setDragDropMode(QAbstractItemView::InternalMove);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDropIndicatorShown(true);
}

void CustomTreeWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void CustomTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    QTreeWidgetItem *targetItem = itemAt(event->position().toPoint());
    if (!targetItem) {
        event->ignore();
        showDropIndicator = false; // Oculta el indicador si no hay target
        viewport()->update();      // Redibuja el widget
        return;
    }

    QRect itemRect = visualItemRect(targetItem); // Obtén el rectángulo visual del targetItem
    QPoint pos = event->position().toPoint();    // Obtén la posición actual del cursor

    // Decide si el indicador debe estar encima, en medio o debajo del target
    if (pos.y() < itemRect.top() + itemRect.height() / 3) {
        // Caso 1: Soltar encima
        dropIndicatorRect = QRect(itemRect.left(), itemRect.top() - 4, itemRect.width(), 8);
    } else if (pos.y() > itemRect.bottom() - itemRect.height() / 3) {
        // Caso 2: Soltar debajo
        dropIndicatorRect = QRect(itemRect.left(), itemRect.bottom() - 4, itemRect.width(), 8);
    } else {
        // Caso 3: Soltar como hijo (en el centro)
        dropIndicatorRect = itemRect.adjusted(4, 4, -4, -4); // Pequeño borde interno
    }

    showDropIndicator = true; // Muestra el indicador
    viewport()->update();     // Redibuja el widget
    event->acceptProposedAction();
}

QTreeWidgetItem *getRootParent(QTreeWidgetItem *item)
{
    while (item && item->parent()) {
        item = item->parent();
    }
    return item;
}

void CustomTreeWidget::dropEvent(QDropEvent *event)
{
    QTreeWidgetItem *targetItem = itemAt(event->position().toPoint());
    QTreeWidgetItem *sourceItem = nullptr;

    QTreeWidget *sourceTree = qobject_cast<QTreeWidget *>(event->source());
    if (sourceTree) {
        sourceItem = sourceTree->currentItem();
    }

    if (!targetItem || !sourceItem) {
        event->ignore();
        return;
    }

    QTreeWidgetItem *parentItem;
    int dropIndex;

    QRect itemRect = visualItemRect(targetItem); // Obtén el rectángulo visual del targetItem
    QPoint pos = event->position().toPoint();    // Obtén la posición actual del cursor

    // Verificar si el componente objetivo permite hijos
    std::string componentTypeStrTarget = targetItem->text(0).toStdString();
    ComponentType typeTarget = stringToComponentType(componentTypeStrTarget);
    Component tempComponent(typeTarget);

    // Verificar para sections
    bool targetIsSection = (componentTypeToString(typeTarget) == "Undefined");
    bool isTopLevel = (targetItem->parent() == nullptr);

    // Verificar si el componente fuente es un Section (Custom Component)
    std::string componentTypeStrSource = sourceItem->text(0).toStdString();
    ComponentType typeSource = stringToComponentType(componentTypeStrSource);
    bool sourceIsSection = (componentTypeToString(typeSource) == "Undefined");
    // qDebug() << "isSection" << isSection;
    // qDebug() << "isTopLevel" << isTopLevel;
    QString logAction;

    // Prevent a Custom Component from being added inside itself
    if (sourceIsSection) {
        QTreeWidgetItem *rootParent = getRootParent(targetItem);
        if (rootParent && rootParent->text(0) == sourceItem->text(0)) {
            qDebug() << "❌ Error: A Custom Component cannot be nested inside itself!";
            event->ignore();
            return;
        }
    }

    if ((tempComponent.isAllowingItems() || (targetIsSection && isTopLevel))
        && pos.y() > itemRect.top() + itemRect.height() / 3
        && pos.y() < itemRect.bottom() - itemRect.height() / 3) {
        if (targetIsSection && isTopLevel && targetItem->childCount() >= 1) {
            qDebug() << "❌ No se puede anidar más de un componente en un Section toplevel.";
            event->ignore();
            return;
        }

        // Caso 3: Insertar como hijo
        parentItem = targetItem;
        dropIndex = 0; // Insertar como el primer hijo

        // Log para "nest-tag"
        QString sourceTagName = sourceItem->text(0); // Nombre del componente arrastrado
        QString targetTagName = targetItem->text(0); // Nombre del componente destino

        if (sourceTagName.toStdString() == "Vertical Layout"
            || sourceTagName.toStdString() == "Horizontal Layout")
            loggerJson.logAction("use-layout-tailwind",
                                 "Layout añadido al árbol de componentes: "
                                     + sourceTagName.toStdString());
        loggerJson.logAction("add-new-tag",
                             "Etiqueta añadida al árbol de componentes: "
                                 + sourceTagName.toStdString());
        loggerJson.logAction("nest-tag",
                             "Etiqueta " + sourceTagName.toStdString() + " anidada dentro de "
                                 + targetTagName.toStdString());
        logAction = "nest-tag";

    } else if (pos.y() < itemRect.top() + itemRect.height() / 3) {
        // Caso 1: Insertar encima
        parentItem = targetItem->parent() ? targetItem->parent() : invisibleRootItem();
        dropIndex = parentItem->indexOfChild(targetItem);

        // Validación extra para no permitir múltiples hijos en Section toplevel
        if (parentItem && parentItem != invisibleRootItem()) {
            std::string parentComponentTypeStr = parentItem->text(0).toStdString();
            ComponentType parentType = stringToComponentType(parentComponentTypeStr);
            bool parentIsSection = (componentTypeToString(parentType) == "Undefined");
            bool parentIsTopLevel = (parentItem->parent() == nullptr);

            if (parentIsSection && parentIsTopLevel && parentItem->childCount() >= 1) {
                qDebug() << "❌ No se puede agregar más de un hijo a un Section toplevel.";
                event->ignore();
                return;
            }
        }
    } else if (pos.y() > itemRect.bottom() - itemRect.height() / 3) {
        // Caso 2: Insertar debajo
        parentItem = targetItem->parent() ? targetItem->parent() : invisibleRootItem();
        dropIndex = parentItem->indexOfChild(targetItem) + 1;

        if (parentItem && parentItem != invisibleRootItem()) {
            std::string parentComponentTypeStr = parentItem->text(0).toStdString();
            ComponentType parentType = stringToComponentType(parentComponentTypeStr);
            bool parentIsSection = (componentTypeToString(parentType) == "Undefined");
            bool parentIsTopLevel = (parentItem->parent() == nullptr);

            if (parentIsSection && parentIsTopLevel && parentItem->childCount() >= 1) {
                qDebug() << "❌ No se puede agregar más de un hijo a un Section toplevel.";
                event->ignore();
                return;
            }
        }
    } else {
        event->ignore();
        return;
    }

    // Mover el elemento
    if (parentItem && sourceItem) {
        event->acceptProposedAction();
        emit itemDropped(parentItem, sourceItem, dropIndex);

        // Log para "add-new-tag" (cuando no es un nido)
        if (parentItem != targetItem) {
            QString tagName = sourceItem->text(0); // Nombre del tag
            loggerJson.logAction("add-new-tag",
                                 "Etiqueta añadida al árbol de componentes: "
                                     + tagName.toStdString());
            logAction = "add-new-tag";
        }
    }
    if (!logAction.isEmpty()) {
        loggerJson.logAction(logAction.toStdString(), "Componente movido");
    }
    // Oculta el indicador después del drop
    showDropIndicator = false;
    viewport()->update();
}

void CustomTreeWidget::paintEvent(QPaintEvent *event)
{
    QTreeWidget::paintEvent(event); // Llama al comportamiento predeterminado

    // Dibuja el indicador solo si está habilitado
    if (showDropIndicator) {
        QPainter painter(viewport());
        painter.setPen(QPen(Qt::blue, 2, Qt::SolidLine));
        // painter.setBrush(Qt::NoBrush);
        painter.drawRect(dropIndicatorRect); // Dibuja el rectángulo del indicador
    }
}
