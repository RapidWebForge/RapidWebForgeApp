#include "customtreewidget.h"

#include <QDropEvent>
#include <QMessageBox>
#include "../../models/component-type/componenttype.h"
#include "../../models/component/component.h"

CustomTreeWidget::CustomTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{}

void CustomTreeWidget::dropEvent(QDropEvent *event)
{
    QTreeWidgetItem *targetItem = itemAt(event->position().toPoint());
    QTreeWidgetItem *draggedItem = currentItem();

    if (targetItem && draggedItem) {
        std::string targetComponentTypeStr = targetItem->text(0).toStdString();
        ComponentType targetComponentType = stringToComponentType(targetComponentTypeStr);

        // Verificar si el componente destino permite anidar
        Component targetComponent(targetComponentType);
        if (!targetComponent.isAllowingItems()) {
            QMessageBox::warning(this,
                                 "Invalid Operation",
                                 "This component does not allow nesting.");
            event->ignore(); // Cancelar el evento de drop
            return;
        }

        emit itemDropped(targetItem, draggedItem); // Emitir la señal personalizada si es válido
    }

    QTreeWidget::dropEvent(event); // Continuar con el evento de drop si está permitido
}
