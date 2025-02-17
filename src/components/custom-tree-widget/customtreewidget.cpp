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

ActionLoggerJson loggerJson("resources/logs/user_actions.json");

CustomTreeWidget::CustomTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
    , loggerJson("resources/logs/user_actions.json") // Cambiar la ruta al archivo JSON

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

    // Verificar si el componente permite hijos
    std::string componentTypeStr = targetItem->text(0).toStdString();
    ComponentType type = stringToComponentType(componentTypeStr);
    Component tempComponent(type);

    // Check for sections
    bool isSection = (componentTypeToString(type) == "Undefined");
    bool isTopLevel = (targetItem->parent() == nullptr);

    // qDebug() << "isSection" << isSection;
    // qDebug() << "isTopLevel" << isTopLevel;
    QString logAction;

    if ((tempComponent.isAllowingItems() || (isSection && isTopLevel))
        && pos.y() > itemRect.top() + itemRect.height() / 3
        && pos.y() < itemRect.bottom() - itemRect.height() / 3) {
        // Caso 3: Insertar como hijo
        parentItem = targetItem;
        dropIndex = 0; // Insertar como el primer hijo

        // Log para "nest-tag"
        QString sourceTagName = sourceItem->text(0); // Nombre del componente arrastrado
        QString targetTagName = targetItem->text(0); // Nombre del componente destino
        loggerJson.logAction("nest-tag",
                             "Etiqueta " + sourceTagName.toStdString() + " anidada dentro de "
                                 + targetTagName.toStdString());
        emit stepUpdated("nest-tag");
        logAction = "nest-tag";

    } else if (pos.y() < itemRect.top() + itemRect.height() / 3) {
        // Caso 1: Insertar encima
        parentItem = targetItem->parent() ? targetItem->parent() : invisibleRootItem();
        dropIndex = parentItem->indexOfChild(targetItem);
    } else if (pos.y() > itemRect.bottom() - itemRect.height() / 3) {
        // Caso 2: Insertar debajo
        parentItem = targetItem->parent() ? targetItem->parent() : invisibleRootItem();
        dropIndex = parentItem->indexOfChild(targetItem) + 1;
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
            emit stepUpdated("add-new-tag"); // 📢 Aquí se emite la señal después del log
            logAction = "add-new-tag";
        }
    }
    if (!logAction.isEmpty()) {
        loggerJson.logAction(logAction.toStdString(), "Componente movido");

        // 🔍 Verificar si la señal realmente se emite
        qDebug() << "🚀 Emitting stepUpdated with action:" << logAction;
        emit stepUpdated(logAction);
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
