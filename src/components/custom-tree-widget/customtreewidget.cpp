#include "customtreewidget.h"
#include <QApplication>
#include <QDrag>
#include <QDropEvent>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include "../../models/component-type/componenttype.h"
#include "../../models/component/component.h"

CustomTreeWidget::CustomTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDropIndicatorShown(true);
}

void CustomTreeWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QTreeWidget* source = qobject_cast<QTreeWidget*>(event->source());
    qDebug() << "Drag Enter - Source:" << (source ? "TreeWidget" : "Other");
    
    event->acceptProposedAction();
}

void CustomTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    QTreeWidgetItem* targetItem = itemAt(event->position().toPoint());
    if (targetItem) {
        qDebug() << "Can drop at:" << targetItem->text(0);
        event->acceptProposedAction();
    }
}

void CustomTreeWidget::dropEvent(QDropEvent *event)
{
    QTreeWidget* source = qobject_cast<QTreeWidget*>(event->source());
    QTreeWidgetItem* sourceItem = nullptr;
    
    if (source) {
        sourceItem = source->currentItem();
    }
    
    QTreeWidgetItem* targetItem = itemAt(event->position().toPoint());

    // qDebug() << "Drop Event Details:";
    // qDebug() << "- Source Widget:" << (source ? "Valid" : "Invalid");
    // qDebug() << "- Source Item:" << (sourceItem ? sourceItem->text(0) : "null");
    // qDebug() << "- Target Item:" << (targetItem ? targetItem->text(0) : "null");

    if (targetItem && sourceItem) {
        event->acceptProposedAction();

        // Obtener el índice de inserción
        int dropIndex = indexOfTopLevelItem(targetItem);
        if (dropIndex == -1 && targetItem->parent()) {
            dropIndex = targetItem->parent()->indexOfChild(targetItem);
        }

        emit itemDropped(targetItem, sourceItem, dropIndex);
        // qDebug() << "Signal emitted!";
    } else {
        event->ignore();
        qDebug() << "Drop ignored - missing items";
    }
}
