// customtreewidget.cpp
#include "customtreewidget.h"
#include <QApplication>
#include <QDrag>
#include <QDropEvent>
#include <QMimeData>
#include "../../models/component-type/componenttype.h"
#include "../../models/component/component.h"

CustomTreeWidget::CustomTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::InternalMove);
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
        return;
    }

    setDropIndicatorShown(true);
    event->setDropAction(Qt::MoveAction);
    event->accept();
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

    // Verificar si el componente permite hijos
    std::string componentTypeStr = targetItem->text(0).toStdString();
    ComponentType type = stringToComponentType(componentTypeStr);
    Component tempComponent(type);

    if (tempComponent.isAllowingItems()) {
        // Insertar como hijo
        parentItem = targetItem;
        dropIndex = 0; // Insertar como el primer hijo en caso de que sea "en medio"
    } else {
        // Insertar al mismo nivel (como hermano)
        parentItem = targetItem->parent() ? targetItem->parent() : invisibleRootItem();
        dropIndex = parentItem->indexOfChild(targetItem);

        // Si estamos tratando de insertar encima del primer elemento del grupo, ajustamos el índice a 0
        if (dropIndex == 0
            && event->position().toPoint().y()
                   < visualItemRect(targetItem).top() + visualItemRect(targetItem).height() / 2) {
            dropIndex = 0; // Asegura que se inserte al principio
        } else {
            dropIndex += 1; // Inserta después en otros casos
        }
    }

    // Mover el elemento y emitir la señal
    if (parentItem && sourceItem) {
        event->acceptProposedAction();
        emit itemDropped(parentItem, sourceItem, dropIndex);
    }
}
